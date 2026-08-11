# rust-tracker — spike Rust + egui

Tranche verticale expérimentale : réécrire la partie **tracker** (pas la DLL) en
Rust + [egui](https://github.com/emilk/egui), pour évaluer une sortie de Qt.

La DLL `PJ64OoTMMTracker` et `PJ64Injector` ne changent pas : ce spike se
branche sur le **même contrat de mémoire partagée** `PJ64_SHARED_MEM`.

## Ce que ça prouve

Les briques de l'architecture, sans Qt :

1. **Mémoire partagée** — mappe `PJ64_SHARED_MEM` et consomme le ring buffer
   d'`Event` en direct. Voir [`src/shared_mem.rs`](src/shared_mem.rs) : la struct
   C `SharedData` est remirroir en `#[repr(C)]` et lue par **cast direct**, zéro
   marshalling (c'est l'argument clé pour Rust ici).
2. **Navigation complète** — arbre jeux → régions → scènes, sur **toutes** les
   scènes des deux jeux (153 OoT + 154 MM), généré depuis les tables C++.
3. **Rendu de map** — charge l'image de la scène sélectionnée avec pan/zoom.
4. **Overlays cliquables** — les ~7650 objets aux coordonnées réelles, clic =
   toggle « collecté ».
5. **Auto-cochage (phase 1)** — les `Event` de la DLL cochent automatiquement
   les overlays (voir [`src/tracking.rs`](src/tracking.rs)).

## Génération des données (`tools/gen_data.py`)

`src/data.rs` est **généré directement depuis les CSV** (la vraie source, comme
`Resources/Objects/Pool Transform.py`), pas écrit à la main :
`Resources/Objects/pool_*.csv` (objets) + `Resources/Scenes/scenes_*.csv` (scènes).

Le Rust généré **préserve tes noms symboliques** : `mod scenes` / `mod ids`
(les `#define`), et les enums `ObjectType` / `ObjectContext` / `GameLayout` /
`LocType`. Valeurs des symboles lues dans `Headers/Combo/{Scenes,NPC,Items,Objects}.h`.
Couverture : **100 %** des lignes des pools.

Régénérer après une modif d'un CSV :

```bash
python tools/gen_data.py
```

## Correspondance avec le tracker C++

| Spike Rust | Équivalent C++ |
|---|---|
| `shared_mem.rs` | `Sources/UI/MemoryReader.cpp` + struct de `Headers/UI/MemoryReader.h` |
| `data.rs` (généré depuis les CSV) | `OoT/MMObjectScene.cpp` + `Scenes.cpp` |
| `tracking.rs` | `MemoryReader::CheckEvent` + `FindObject` + `CorrectComboItem` |
| `inject.rs` | `MemoryReader::InjectTrackerDLL` / `StartMemoryReader` |
| `scene.rs` | types runtime + `SceneRenderer`/`ObjectRenderer` (couleurs, layout) |
| `main.rs` (`draw_nav`) | arbre des scènes (`SceneItemTree` / `MapTab`) |
| `main.rs` (`draw_map`) | `SceneRenderer` / `ObjectRenderer` |
| `main.rs` (`draw_event_log`) | boucle de polling de `MemoryReader` |

## Prérequis

Rust n'est pas installé sur la machine. Installer la toolchain (MSVC) :

```bash
winget install Rustlang.Rustup
```

Puis, dans un nouveau terminal, s'assurer de la toolchain MSVC :

```bash
rustup default stable-x86_64-pc-windows-msvc
```

## Lancer

Depuis le dossier `rust-tracker/` :

```bash
cargo run --release
```

Les images sont chargées depuis `../Resources/...` (résolu via `CARGO_MANIFEST_DIR`),
donc le dépôt complet doit être présent — pas besoin de copier les assets.

- **Sans PJ64** : « Simuler un évènement » fabrique un vrai ramassage pour un
  objet de la scène courante → tu vois l'auto-cochage. Teste aussi pan/zoom +
  clic sur la map.
- **Avec PJ64** : lance le jeu et injecte la DLL comme d'habitude, puis
  « Reconnecter ». Les objets se cochent tout seuls au fil du jeu.

## Notes techniques

- Le tracker Rust cible **x86_64** ; la DLL/PJ64 sont en **x86 (32 bits)**. C'est
  compatible : `SharedData` ne contient que des champs 32 bits sans pointeur,
  donc le layout est identique entre les deux bitness. **Garder cette struct
  sans pointeur** est la seule contrainte à respecter côté DLL.
- Aucune dépendance `windows-sys` : les 4 appels kernel32 nécessaires sont
  déclarés en `extern "system"` bruts (layout garanti stable, rien à compiler).

## État des étapes

- [x] Mémoire partagée live (ring buffer d'Event)
- [x] Génération des données **depuis les CSV** (`tools/gen_data.py`)
- [x] Constantes/enums symboliques préservés (`scenes::`, `ids::`, `ObjectType`…)
- [x] Navigation toutes scènes (arbre jeux → régions → scènes)
- [x] Rendu map + overlays cliquables (pan/zoom)
- [x] **Auto-injection** — le tracker lance `PJ64Injector.exe` tout seul
  (démarrage / bouton / re-tente toutes les ~0,7 s, détecte la fermeture de
  PJ64). Voir [`src/inject.rs`](src/inject.rs).
- [x] **Milestone B — phase 1** : overlays *direct* (chest / collectible /
  stray-fairy) et *extended-flag* (grass, pot, butterfly…).
- [x] **Milestone B — phase 2** : types à remap de scène (GS / shop / npc / cow /
  scrub / silver-rupee / fish) via **recherche globale `(type, id)`** — l'objet
  du pool porte déjà sa scène, donc aucun portage des `switch GetSceneX`. Filtre
  de layout (base, hors MQ/JP). Tout validé par `cargo test`.
- [x] Chemin « nothing » (grass/pots vides) via `ParseKey`
- [x] ROM stable/dev — décalage NPC `ResolveRawOoTNpcID`, version depuis le spoiler
- [x] **Spoiler log** — glisser-déposer un spoiler : item par emplacement (affiché
  au survol) + version ROM. Match global par `Location` unique. Voir
  [`src/spoiler.rs`](src/spoiler.rs).
- [x] **Save/load** — auto-sauvegarde/-chargement de l'état collecté, clé par
  `Location` (résilient à la régénération des données).
- [x] **Layout MQ/JP** — lu dans les réglages du spoiler (`Master Quest
  Dungeons: all|none|liste`), filtre de layout par-scène (`tracking::object_active`).
- [x] **Filtre de contexte** — menu Tout/Enfant/Adulte/Hiver/Printemps.
- [x] **Rendu par salle** des donjons (`RoomRenderer`) — table `OOT_ROOMS`/
  `MM_ROOMS` générée ; sélecteur de salle + filtre des objets par `RoomID`.
- [~] **Entrances** (`EntranceHelper`) + **GPS**
  - [x] Phase 1 — données générées (`OOT/MM_ENTRANCES`) + affichage sur la
    **minimap** (toggle vue **Objets/Entrées** ; `SceneDef` a `image_rel`
    artistique **et** `minimap_rel`), losanges + nom au survol
  - [x] Phase 2a — décodage des messages IN/OUT (`entrance.rs`, mirror de
    `SetMessage`/`LookupEntrance`) + entrées **visitées** surlignées en vert
    sur la minimap, persistées
  - [x] Phase 2b — graphe de connexions (X→Y, chemin normal, persisté) ; cas
    spéciaux (grottos par position, songs, spawns, morts) restent approximés
  - [x] Phase 3 — GPS (`gps.rs`, BFS sur le graphe d'entrées + connexions
    découvertes ; coûts mesurés = raffinement)
- [x] **Onglet progression** — items obtenus (objets collectés × spoiler)
- [x] **Multiworld** — routage « Player N » affiché (tracking par monde =
  raffinement)
- [x] **Icônes réelles** — `ICON_PATHS` généré depuis `Icons.cpp`, rendues sur
  la map (fallback pastille colorée)
