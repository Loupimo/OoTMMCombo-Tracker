# rust-tracker — le tracker OoTMM (Rust + egui)

Application **Rust + [egui](https://github.com/emilk/egui)** : c'est le tracker OoTMM
**livré depuis la 3.0.0**, réécriture de l'ancien tracker Qt/C++ (désormais rangé
dans `C++-Tracker/`). L'exécutable produit s'appelle `OoTMMCombo-Tracker`.

Le tracker se branche sur la DLL de hook `PJ64OoTMMTracker` (toujours en C++, dans
`C++-Tracker/PJ64Tracking/`) via le **même contrat de mémoire partagée**
`PJ64_SHARED_MEM`. Il n'y a **plus d'injecteur externe** : le tracker copie la DLL
dans le dossier `Plugin/` de Project64 et demande à PJ64 de la charger (Ctrl+T).

## Compiler & lancer

Prérequis : la toolchain Rust MSVC.

```bash
winget install Rustlang.Rustup
rustup default stable-x86_64-pc-windows-msvc
```

Puis, depuis `rust-tracker/` :

```bash
cargo run --release      # lancer le tracker
cargo build --release    # produire target/release/OoTMMCombo-Tracker.exe
cargo test               # tests unitaires
```

Les assets (maps, icônes) et les fichiers propres au tracker (réglages, sauvegardes,
sidecar spoiler, dossier `autosave/`) sont résolus au runtime par `scene::base_dir()`
/ `scene::data_dir()` : à côté de l'`.exe` en build déployé, ou depuis l'arbre source
en `cargo run`. Le dépôt complet doit donc être présent en développement.

## Architecture (carte des modules)

### Cœur / interface
| Module | Rôle |
|---|---|
| `main.rs` | point d'entrée, struct `TrackerApp`, déclaration des modules |
| `state.rs` | état de `TrackerApp` : persistance, gestion des évènements live, boucle `eframe::App` |
| `scene.rs` | types runtime au-dessus des tables générées (`Game`, `LiveScene`) + résolution des chemins d'assets |
| `ui/` | panneaux egui : `menu`, `map`, `launch`, `progression`, `entrance`, `gps`, `settings_window`, `kbdnav` |
| `dialog.rs` | dialogues de fichiers natifs (ouvrir / enregistrer) |
| `i18n.rs` | internationalisation EN/FR (`I18n`, locales), `AppSettings` |

### Connexion au jeu (hook / mémoire partagée)
| Module | Rôle |
|---|---|
| `shared_mem.rs` | mémoire partagée Win32 : `SharedData` `#[repr(C)]` + ring buffer d'`Event`, lu par cast direct |
| `poller.rs` | thread de fond qui possède le lien mémoire partagée et orchestre le cycle de vie (chargement, attente, déchargement propre) |
| `inject.rs` | chargement de la DLL : copie dans `Plugin/` + Ctrl+T (plus d'injecteur externe) |
| `tracking.rs` | résolution `Event` → objet du pool, remap d'ids d'item, détection de version ROM |

### Données de seed (spoiler / réglages / patch / logique)
| Module | Rôle |
|---|---|
| `spoiler.rs` | parsing du spoiler log (placements par monde, version, MQ) |
| `settings.rs` | réglages ROM parsés du spoiler, objets exclus, ids progressifs / partagés, remaps d'entrances |
| `patch.rs` | chargement d'un patch `.ootmm` (archive zip + `meta.json`) |
| `logic/` | moteur d'accessibilité : `mod` (`solve_world`), `inputs` (inventaire par joueur), `eval` (bytecode), `solve` (point fixe) |

### Fonctionnalités
| Module | Rôle |
|---|---|
| `progression.rs` | onglet progression (`Dashboard`), résolution `find_item_id`, familles progressives |
| `entrance.rs` | `EntranceHelper` : décodage des messages d'entrance IN/OUT, cas spéciaux (grottos, songs, spawns, morts) |
| `gps.rs` | routage GPS (BFS sur le graphe d'entrances + connexions découvertes, cross-game) |
| `qtsave.rs` | import des sauvegardes binaires Qt `.trck` |
| `multi/` | client multijoueur (fork du multi-client de Nax) |
| `multi_r4/` | client multijoueur r4 (builds dev : pipe nommé + WAL) |

### Données & build
| Module | Rôle |
|---|---|
| `data/` | tables statiques immuables **générées** (voir plus bas) |
| `build.rs` | embarque `Logo.ico` comme ressource PE (icône Explorer / barre des tâches) |

## Génération des données

La quasi-totalité de `src/data/*.rs` est **générée**, pas écrite à la main :

- `python tools/gen_data.py` — tables de jeu depuis les sources C++ / CSV :
  `consts.rs` (scenes / ids / enums symboliques), `oot_items.rs` + `mm_items.rs`
  (pools d'objets), `oot_world.rs` + `mm_world.rs` (scènes / salles / entrances),
  `misc.rs` (`ItemDef`, `ITEM_BY_NAME_LC`, méta-réglages…), `prog.rs` (dashboard +
  `PROGRESSIVE_FAMILIES`).
- `python tools/gen_logic.py` — `data/logic.rs` : compile les règles d'accessibilité
  du dossier `Logic/` (racine du dépôt) en un graphe de régions + bytecode
  (`Op` / `EXPRS`).

Le Rust généré **préserve les noms symboliques** (`scenes::`, `ids::`, `iid::`, enums
`ObjectType` / `GameLayout` / …). Régénérer après une modif d'une source :

```bash
python tools/gen_data.py
python tools/gen_logic.py
```

**Écrit à la main** (n'est PAS régénéré) : `progression.rs`, `state.rs`, `settings.rs`,
`i18n.rs`, `spoiler.rs`, `tracking.rs`, `entrance.rs`, `gps.rs`, `logic/*` (sauf
`data/logic.rs`), `ui/*`, `locales/*.toml`. Les correctifs faits à la main (alias de
noms d'items, familles progressives supplémentaires…) survivent donc à une
régénération.

## Sauvegardes

- Format **XML lisible et éditable à la main** (`<tracker version="6">`), groupé par
  scène. Chaque check est chargé sur son **identité numérique stable** (l'`object_id`
  au sein de sa scène, départagé au besoin par `type` + `layout`), avec la chaîne
  `Location` en **repli** — une sauvegarde survit donc au renommage d'une location
  d'une version à l'autre. Les items placés portent aussi leur `id` d'item (stable),
  et les liens d'entrance enregistrent le **nom entier résolu** de la sortie
  (« scène - côté »). Voir `state.rs` (`render_save_xml` / `parse_save_xml`).
- **Autosave par seed** : un fichier par seed sous `autosave/<seed>.xml` (hash de la
  première ligne du spoiler), `empty.xml` en l'absence de spoiler. Charger un spoiler
  d'une autre seed bascule sur son propre fichier.
- Import rétro-compatible : anciens `.trck` binaires Qt (`qtsave.rs`) et ancien format
  texte Rust.

## Notes techniques

- Le tracker est **x86_64** ; la DLL / PJ64 sont **x86 (32 bits)**. C'est compatible
  car `SharedData` ne contient que des champs 32 bits **sans pointeur** → layout
  identique entre les deux bitness. **Garder cette struct sans pointeur** est la seule
  contrainte à respecter côté DLL (vérifié par le test `shared_data_layout_matches_cpp`).
- Aucune dépendance `windows-sys` : les appels kernel32 nécessaires sont déclarés en
  `extern "system"` bruts (`shared_mem.rs`, `inject.rs`).
- **Project64 doit être en mode fenêtré** au démarrage du tracking : le chargement du
  plugin passe par un Ctrl+T envoyé à la fenêtre PJ64, avalé en plein écran.

## Tests

```bash
cargo test
```

Couvre entre autres : layout mémoire partagée ↔ C++, parsing spoiler (mono / multi,
versions), résolution d'items (familles progressives, alias clocks / rusty keys),
réglages & exclusions, solveur d'accessibilité (monotonie, routage multiworld),
round-trip de sauvegarde XML, et i18n (toutes les clés présentes dans les deux
locales).

## Correspondance avec le tracker C++ (`C++-Tracker/`)

Utile pour retrouver l'origine d'un comportement (les chemins sont relatifs à
`C++-Tracker/`).

| Rust | Équivalent C++ |
|---|---|
| `shared_mem.rs` | `Sources/UI/MemoryReader.cpp` + `Headers/UI/MemoryReader.h` |
| `poller.rs` | boucle de polling de `MemoryReader` |
| `inject.rs` | `MemoryReader::InjectTrackerDLL` / `StartMemoryReader` (sans injecteur) |
| `tracking.rs` | `MemoryReader::CheckEvent` + `FindObject` + `CorrectComboItem` |
| `data/` (généré) | `Sources/Combo/*` (`OoT`/`MMObjectScene.cpp`, `Scenes.cpp`, `Items.cpp`, `Entrances.cpp`…) |
| `progression.rs` | `Sources/UI/ProgressionEntry.cpp` |
| `settings.rs` | `Sources/UI/Settings.cpp` / `SettingsTab.cpp` |
| `entrance.rs` | `Sources/Combo/Entrances.cpp` (`EntranceHelper`) |
| `ui/map.rs` | `SceneRenderer` / `ObjectRenderer` |
| `multi/` | `Sources/Multi` (fork du multi-client) |
