# Régénération des données (objets, XflagID, logique)

Manuel des scripts qui transforment les données OoTMM en tables du tracker **Qt (C++)**
et **Rust**. À lire quand tu mets à jour les XML `New/**`, quand OoTMM sort une version
qui bouge des checks, ou quand tu changes le rendu (positions / icônes / noms / layout).

---

## 1. Le pipeline en un coup d'œil

```
                 OoTMM checkout (data/checks/**)          <- source amont (une par version de ROM)
                          │
                          │  gen_xflags.py --stamp-csv     (recalcule le XflagID global par Location)
                          ▼
   Objects/pool_oot.csv · pool_mm.csv   ── colonne xflag_id ──┐
                          │                                    │
   Objects/New/**.xml  (schéma OoTMM data/checks + couche render : xyz/icon/layout/context)
                          │                                    │
        ┌─────────────────┴───────────────┐                   │
        │ gen_objects.py --emit           │  augment.py --all │ (ajoute la couche render
        ▼ (offline: lit xflag_id du pool) │                   │  aux XML identity-only)
  Sources/Combo/OoTObjectScene.cpp        │                   │
  Sources/Combo/MMObjectScene.cpp   ◄─────┘                   │
        (tracker Qt)                                          │
                          │  gen_data.py (importe gen_objects.build_objects(), offline)
                          ▼
   rust-tracker/src/data/*.rs  (oot_items, mm_items, mod, …)  ◄── même pool, mêmes lignes
                          │  gen_logic.py (appelé par gen_data)
                          ▼
   rust-tracker/src/data/logic.rs   (graphe de logique, depuis Logic/**.yml)
```

**Deux identifiants à ne pas confondre :**

| | Source | Où c'est stocké | Change quand… |
|---|---|---|---|
| **XflagID** (compact, ROMs > v32.3) | compteur **global** rejoué sur le checkout OoTMM par `gen_xflags` | colonne `xflag_id` des pool CSV | OoTMM **ajoute / retire / réordonne** des checks en amont |
| **ObjectID legacy** (ROMs ≤ v32.3) | composé depuis `<xflag slice setup actor>` (ou pris du pool) | dérivé au build | tu changes l'identité `<xflag>` d'un check dans le XML |

> Le côté **Rust lit toujours le `xflag_id` du pool** (mode offline, `gen_data.py` n'accepte pas
> de `<ootmm_root>`). Donc pour propager de **nouvelles valeurs** de XflagID au Rust, il faut
> **d'abord re-stamper le pool** avec `gen_xflags`.

---

## 2. Les scripts

| Script | Emplacement | Rôle |
|---|---|---|
| `gen_xflags.py` | `C++-Tracker/Resources/` | Rejoue le compteur XflagID global sur un checkout OoTMM ; `--stamp-csv` écrit la colonne `xflag_id` des pool CSV. `<ootmm_root>` accepte un dossier local **ou** une URL GitHub (téléchargée+cachée, voir §5). |
| `augment.py` | `C++-Tracker/Resources/` | Ajoute la **couche render** (xyz/icon/layout/context) aux XML `New/**` identity-only, en joignant le pool par Location. `--all` = tout sauf le set HAND. Idempotent. |
| `gen_objects.py` | `C++-Tracker/Resources/` | Génère `OoT/MMObjectScene.cpp` (Qt) depuis `New/**`. `--emit` écrit, `--diff` compare au pool, **`--sync-pool`** back-fill le pool avec les checks du XML absents du pool (voir Cas D). Source unique des objets pour Qt **et** Rust (`build_objects()`). |
| `gen_data.py` | `rust-tracker/tools/` | Génère `rust-tracker/src/data/*.rs` (objets via `gen_objects.build_objects()`, + items/scènes/entrées/progression). Appelle `gen_logic`. |
| `gen_logic.py` | `rust-tracker/tools/` | Compile `Logic/**.yml` → `src/data/logic.rs`. |
| `gen_setting_options.py` | `rust-tracker/tools/` | Extrait les **listes d'options par paramètre** de `packages/core/src/settings/data.ts` (OoTMM) → snapshot `tools/ootmm_settings.json`. Lu **hors-ligne** par `gen_data.py` pour peupler `SettingMeta.options`. Accepte un `data.ts`, une racine OoTMM, ou télécharge depuis master. À relancer après un update OoTMM (voir §5). |
| `Position Finder.py` | `C++-Tracker/Resources/` | Utilitaire : extrait des positions xyz depuis une image de map color-codée (aide à remplir le rendu). |
| `ItemShift.py` | `C++-Tracker/Resources/` | Utilitaire ad-hoc : renumérote des IDs d'items selon un ordre donné (décalages dev/stable). |

> `python` = ton Python 3. Le pipeline principal (`gen_xflags`, `gen_objects`, `augment`,
> `gen_data`, `gen_logic`) n'utilise **que la stdlib** (aucun paquet à installer). Seuls les
> utilitaires en ont besoin : `Position Finder.py` → `opencv-python`+`numpy`, `ItemShift.py` →
> `pandas`. Les CSV/XML sont en **CRLF** : les scripts le préservent, ne les convertis pas en LF.

---

## 3. Recettes (les 3 cas courants)

### Cas A — je change seulement le **rendu** d'un ou plusieurs checks
*(déplacer un marqueur, changer l'icône / le nom / le layout / le context / la room — sans*
*ajouter/retirer de check ni renommer une Location, donc les XflagID ne bougent pas)*

```bash
# 1. édite le(s) fichier(s) C++-Tracker/Resources/Objects/New/**/*.xml
cd C++-Tracker/Resources
python gen_objects.py --emit          # → OoT/MMObjectScene.cpp (Qt), offline (pool)
cd ../../rust-tracker
python tools/gen_data.py              # → src/data/*.rs (+ logic.rs)
cargo build && cargo test
```

C'est le cas de la retouche `twin_islands.xml` : pas besoin de `gen_xflags` tant que ce sont
les mêmes checks aux mêmes Locations.

### Cas B — **mise à jour OoTMM** (une nouvelle version bouge / ajoute des checks → les XflagID changent)
*Il te faut la **version OoTMM qui correspond à ta ROM**. `<ootmm_root>` peut être :*
- *un **dossier local** (la racine du repo OoTMM : contient `data/checks/` et `data/defs/scenes.yml`), ou*
- *une **URL GitHub** / `owner/repo[@ref]` : téléchargée une fois puis mise en cache (voir §5).*

```bash
cd C++-Tracker/Resources
# 1. recalcule le XflagID global et ré-écris la colonne xflag_id des pools
python gen_xflags.py <ootmm_root> --stamp-csv Objects/pool_oot.csv Objects/pool_mm.csv
#    ex. local  : python gen_xflags.py D:/dev/OoTMM --stamp-csv Objects/pool_oot.csv Objects/pool_mm.csv
#    ex. GitHub : python gen_xflags.py https://github.com/OoTMM/OoTMM/tree/master --stamp-csv Objects/pool_oot.csv Objects/pool_mm.csv
#    (affiche "N/M rows stamped" par fichier ; vérifie que M ≈ total attendu)

# 2. si OoTMM a ajouté des fichiers de checks identity-only, ajoute-leur la couche render
python augment.py --all               # idempotent ; saute le set HAND (voir §5)

# 3. régénère les objets Qt (offline : relit le pool tout juste re-stampé)
python gen_objects.py --emit
#    → réconcilie à la main le "scene registration" affiché (voir §5)

# 4. régénère le Rust (relit le même pool re-stampé)
cd ../../rust-tracker
python tools/gen_data.py
cargo build && cargo test
```

> Variante : `python gen_objects.py <ootmm_root> --emit` lit les XflagID **en direct** du checkout
> au lieu du pool. Mais comme le Rust, lui, lit toujours le pool, **re-stampe quand même le pool**
> (étape 1) pour que Qt et Rust restent cohérents.

### Cas C — j'ajoute un **nouveau fichier** OoTMM identity-only
Copie le `.xml` OoTMM sous `New/**`, puis :
```bash
cd C++-Tracker/Resources
python augment.py --all               # ajoute la couche render par jointure pool (Location)
# … puis complète à la main les checks absents du pool (xyz = "0;0;0", name à remplir)
python gen_objects.py --emit
cd ../../rust-tracker && python tools/gen_data.py && cargo build && cargo test
```

### Cas D — protéger de nouveaux checks (gossips) contre l'écrasement
Le **pool CSV est le stockage durable** de la couche render (xyz/nom/icône/layout). Un check que
tu ajoutes *seulement* dans le XML (typiquement une nouvelle gossip) est perdu dès que tu lances
`augment.py --all` (qui reconstruit le XML depuis le pool) **ou** que tu écrases le XML par une copie
fraîche du repo OoTMM. Pour le rendre permanent, recopie-le du XML vers le pool :

```bash
# 1. renseigne les coordonnées / le nom des nouveaux checks dans le(s) New/**/*.xml
cd C++-Tracker/Resources
python gen_objects.py --sync-pool                 # DRY-RUN : liste ce qui serait ajouté au pool
python gen_objects.py <ootmm_root> --sync-pool --write   # applique (avec checkout = vrais XflagID)
#   sans <ootmm_root> : écrit xflag_id=0xFFFF, à compléter ensuite avec gen_xflags --stamp-csv
```

- `--sync-pool` n'ajoute que les Locations **absentes** du pool ; il **ne met pas à jour** une
  ligne existante. ⇒ renseigne les coordonnées dans le XML **avant** de synchroniser (sinon la ligne
  part avec `xyz=0;0;0` et un re-sync ne la corrigera pas).
- Les colonnes `requierements` / `tooltip` (logique/hint) sont laissées vides ; les lignes sont
  ajoutées en fin de fichier, en CRLF, dans l'ordre de colonnes du pool.
- Ordre conseillé après ajout : `--sync-pool --write` → (`gen_xflags --stamp-csv` si offline) →
  `gen_objects --emit` → `gen_data.py`. Là seulement `augment.py --all` est sans danger.

### Juste le graphe de logique (OoTMM a changé `Logic/**`)
`gen_data.py` régénère déjà `logic.rs`. Pour ne faire QUE ça :
```bash
cd rust-tracker && python tools/gen_logic.py
```
Rafraîchis `rust-tracker/tools/tricks.tsv` si OoTMM ajoute des tricks (le spoiler liste les tricks
par nom d'affichage, la logique par id).

---

## 4. Vérifier que tout est cohérent

```bash
# Qt : comparer les lignes générées au pool (aucun emit)
cd C++-Tracker/Resources && python gen_objects.py --diff

# Rust : build + tests (99 attendus verts)
cd ../../rust-tracker && cargo build && cargo test
```

Signaux à lire dans la sortie :
- `gen_objects` : `--emit` **refuse** s'il reste des fichiers non convertis (`REFUSING --emit …`) →
  convertis-les (Cas C) ou `--force` (génère un cpp **partiel**, à éviter).
- `gen_data` : `objects : OoT=… MM=…` et surtout **aucun** `WARN unresolved …`.
  `WARN unresolved object-ids` = un symbole d'`Items.h`/`NPC.h` manque ; `WARN unresolved scenes`
  = un `id=` de scène inconnu de `Scenes.h`.
- `gen_logic` : `locations : logic=… tracker=… matched=…` (le join objet↔logique).
- Les tests `object_location_coverage` et `out_of_pool_checks_have_no_logic_rule` cassent si un
  **renommage** casse le join Location↔logique — c'est là qu'il faut regarder en cas d'update.

---

## 5. Pièges & détails

- **`<ootmm_root>` obligatoire pour changer les valeurs XflagID.** Sans lui, tout tourne en
  *offline* et réutilise le `xflag_id` du pool. C'est voulu (immunise contre les décalages tant
  qu'on ne met pas à jour OoTMM), mais ça veut dire : **une mise à jour OoTMM = re-stamper le pool
  d'abord** (Cas B, étape 1), sinon les nouveaux XflagID n'arrivent ni au Qt ni au Rust.
- **`<ootmm_root>` local OU URL GitHub.** `gen_xflags.py` (et donc `gen_objects.py <ootmm_root>`)
  accepte : un dossier local, une URL `https://github.com/OWNER/REPO[/tree/REF]`, ou `OWNER/REPO[@REF]`
  (`REF` = branche / tag / commit, défaut `master`). L'URL télécharge **une fois** l'archive source
  et n'en extrait que le sous-dossier `data/`, mis en cache sous
  `…/Temp/ootmm_checks_cache/<owner>_<repo>_<ref>/` (réutilisé aux runs suivants ; supprime ce
  dossier pour forcer un re-téléchargement). ⚠️ Choisis le **REF qui correspond à ta ROM** : le HEAD
  de `master` peut avoir des XflagID différents d'une ROM plus ancienne. (Le téléchargement tire
  l'archive complète du repo mais ne garde que `data/` sur le disque.)
- **Set HAND de `augment.py` = VIDE.** Il exclut les fichiers dont la couche render n'est PAS
  reconstructible depuis le pool (à maintenir à la main). Il n'y en a plus aucun : les anciens
  exemples (`oot/special.xml`, `oot/overworld/kakariko.xml`, `oot/dungeons_mq/bottom_of_the_well_mq.xml`)
  sont désormais entièrement adossés au pool, donc `augment --all` les régénère comme les autres. Un
  fichier n'apparaît « un-converted » (`gen_objects` **liste lesquels** désormais) que s'il n'a pas
  encore de `<scene_rendering>` → `augment --all` le corrige (join pool). N'ajoute une entrée dans
  `HAND` que si un fichier reçoit des données render qui n'existent que dans le XML (jamais dans le pool).
- **Drift de registration** (affiché par `gen_objects --emit`) : quand une scène devient non-vide
  (ou l'inverse), le script te dit quoi corriger **à la main** dans `Objects.cpp` /
  `Headers/Combo/OoTObjectScene.h` : retirer un `CreateEmptyScene(...)`, ajouter un
  `extern ObjectInfo* <scene>SceneObjects;`, ou ajouter `CreateSceneObjects(<scene>)` à l'agrégat.
- **`gen_objects` ne convertit que les XML contenant `<scene_rendering>`.** Un fichier OoTMM brut
  (identity-only) est *skippé* tant qu'il n'est pas augmenté → passe par `augment.py` d'abord.
- **Le Rust n'a pas besoin de render-dups.** Le runtime filtre par `render_scene`, donc un check =
  une seule `ObjectDef` (contrairement au cpp Qt qui duplique dans la scène de rendu). C'est normal
  que le compte d'objets Rust (OoT 3318 / MM 2929) soit inférieur à l'ancien pool.
- **XflagID par Location.** Le stamping se fait sur la Location (clé globalement unique). Un check
  absent du pool ⇒ `xflag_id = 0xFFFF` (non tracké en système compact) : à compléter dans le pool
  puis re-stamper.
- **Gossips & unreachables** n'ont pas de règle de logique (normal) : exclus du test de couverture.
- **Objets versionnés legacy/new (colonne `system` du pool).** Quand OoTMM change la *représentation*
  d'un check entre versions (ex. l'herbe du crawl de Cocorico : l'ancien split par ère « Child/Adult
  Near Crawl 7 & 8 » fusionné en « Near Crawl 1 & 2 » agnostiques d'ère par le nouveau format de
  checks), les deux formes coexistent : l'ancienne sur les ROMs `<= v32.3`, la nouvelle sur `> v32.3`.
  - **Où c'est écrit :** la **colonne `system` du pool** (`legacy` / `new` ; vide = `any`) est le
    stockage durable. `augment.py` la recopie en **attribut `system="…"` sur le `<xflag>`** (juste après
    `location`) → le XML est **auto-documenté** (tu vois d'un coup d'œil qu'une location est legacy) et
    hand-éditable. `gen_objects` lit cet **attribut XML en priorité**, avec repli sur le pool par
    Location (`load_pool_system`) pour un XML pas encore augmenté ; `gen_data` reçoit la même valeur.
    Le token est mappé via `SYSTEM_ENUM` (`gen_objects.py`) vers l'enum `ObjSystem`, stampé sur chaque
    objet Qt **et** Rust. ⇒ pour tagger un check : mets `legacy`/`new` dans la colonne `system` du pool
    (ou directement l'attribut `system=` sur le `<xflag>`), puis `augment` + `gen_objects` + `gen_data`.
  - **`augment.py` ré-injecte les checks legacy** absents du XML source (cas typique : tu écrases le
    XML par une copie fraîche d'OoTMM qui ne les a plus). Pour tout row du pool `system ∈
    AUGMENT_SYNTH_SYSTEMS` (= `{"legacy"}`) dont la Location manque, il synthétise le `<xflag>` (identité
    reconstruite depuis l'id du pool, `rendering_option` selon le `context` du pool). Idempotent. Donc
    **`augment --all` ne perd plus les legacy** — c'est ça qui remplace l'ancienne fragilité.
  - Gate runtime : `UsesLegacyXflags()` (C++ `ObjectInfo::HasCorrectLayout`) / `object_active` (Rust,
    global `USES_LEGACY`). Gate **affichage ET résolution** → une paire legacy/new partageant un
    ObjectID ne matche jamais les deux.
  - **Nouveau tier de version** (ex. si v33 rebouge un check) : ajoute le token à `SYSTEM_ENUM` + une
    variante `ObjSystem` + un cas runtime (aujourd'hui binaire via `UsesLegacyXflags`), et au besoin à
    `AUGMENT_SYNTH_SYSTEMS`.
  - ⚠️ Les fusionnés « new » ne se tracke en système compact que si leur `xflag_id` est stampé dans le
    pool (Cas B) ; sinon ils s'affichent mais la collecte ne les résout pas sur ROM new.
  - Note : la colonne `system` est en fin de pool ; `load_pool` (gen_objects + augment) tolère les
    rows plus courtes (colonne absente → `any`).
- **Acteur « hétérogène » = un `<rendering_option>` par check.** Quand plusieurs checks partagent un
  même `<actor>` mais rendent différemment (cas type : une gossip stone dont le `gossip` (petite fée)
  rend en `fairy` et le `gossip-big` (grande fée) en `fairy_spot`), `augment.py` **ne** met **pas**
  l'acteur sous un seul `<rendering_option>` (il prendrait le type du 1ᵉʳ check). Il donne à chaque
  check son propre `<rendering_option>` **imbriqué dans l'`<actor>`** ; `gen_objects` sait lire cette
  imbrication (l'identité vient de l'`<actor>`/`<match>`, le rendertype du `<rendering_option>` interne).
  Un acteur homogène (fontaines de fées, groupes pot/herbe) reste sous un `<rendering_option>` unique.
  → le rendertype vient du **pool** (`rendertype` = `fairy`/`fairy_spot`) : le corriger là puis
  `augment` + `gen_objects --emit` + `gen_data.py`.
- **Nommage items logique ↔ tracker.** La logique OoTMM nomme certains items autrement que tes
  `#define` (ex. `SONG_STORMS`/`SHARED_SONG_STORMS` côté logique = `SONG_OF_STORMS`/
  `SHARED_SONG_OF_STORMS` côté tracker). L'alias est dans `gen_logic.py` → `LOGIC_ITEM_ALIASES` ;
  ajoute une entrée là si un futur item OoTMM ne résout pas (`WARN unresolved items`).
- **Réglages de filtre C++ → Rust (2 étapes).** `gen_data.py` **parse `Settings.cpp`** (`FilterSettings` /
  `ItemSettings`) et régénère la **métadonnée** Rust (`data/misc.rs` → `FILTER_SETTINGS`/`ITEM_SETTINGS` :
  clé, nom, type, défaut). Donc ajouter un `FilterSetting` en C++ puis relancer `gen_data.py` suffit pour
  que le réglage **existe** côté Rust (parsé du save, valeur par défaut, listé dans l'UI). MAIS la **logique
  de filtrage** (quel objet le réglage cache) n'est **pas** générée : c'est du code à mirrorer **à la main**
  dans `rust-tracker/src/settings.rs` (`apply_oot` / `apply_mm`, `match o.render_type { … }`, miroir de
  `ApplyOoT/MMSettingsToFilter`). Le **nom** du réglage ne passe pas par les locales (affiché depuis
  `SettingMeta.name`, anglais) → rien à traduire pour le titre. Ex. gossips : `shuffleGossip[Big]Fairies{Oot,Mm}`.
- **Options par paramètre alignées sur OoTMM (Rust).** L'éditeur ROM propose, par réglage, les **vrais choix
  OoTMM** (ex. une clé → « Own Dungeon / Anywhere / Removed / Vanilla », un cassable → « None / All /
  Overworld / Dungeons only ») au lieu d'une liste générique. Ces choix viennent du snapshot
  `tools/ootmm_settings.json` (généré par `gen_setting_options.py`), que `gen_data.py` lit pour remplir
  `SettingMeta.options: &[SettingOption{value,label}]` dans `data/misc.rs`. À l'affichage, chaque `label`
  OoTMM (`o.label`) **passe** par `tr_settings` → ajoute la traduction FR dans `locales/fr.toml`
  `[settings_names]` (ex. `None`=Aucun, `Own Dungeon`=Propre donjon…). Le tracker mappe chaque option vers
  un **bucket** `ShuffleSetting` via `settings::filter_value` : plusieurs options OoTMM peuvent tomber dans le
  même bucket (ex. `ganonBossKey` : `ganon`+`anywhere`→`all`) — c'est voulu, le filtrage est bucket-based.
  Après un update OoTMM : relancer `gen_setting_options.py` puis `gen_data.py`.
- **Réglages « Logique / Accès » (raw_settings, Rust).** La logique d'accessibilité lit **uniquement**
  `Settings::raw_settings` (rempli par `parse_spoiler`) → `logic/inputs.rs`. Les réglages d'accès / conditions
  de victoire (open dungeons, `doorOfTime`, `rainbowBridge`/`moon`/`lacs`, `ganonTrials`…) ne sont **pas** des
  FilterSettings ; ils sont exposés par une page dédiée qui édite **directement `raw_settings`** (la valeur
  brute OoTMM), donc le solveur les honore même sans spoiler (un spoiler chargé écrase via `parse_spoiler`).
  Métadonnée générée : `gen_data.py` émet `ACCESS_SETTINGS` (struct `AccessSetting`, enum `AccessKind`
  Enum/Bool/Set) depuis la **liste curée `ACCESS_KEYS`** + le snapshot `ootmm_settings.json` (type / défaut /
  options+labels). Pour ajouter une clé : l'ajouter à `ACCESS_KEYS` (elle doit être dans `SETTING_KEYS` de la
  logique) puis relancer `gen_data.py`.
- **Réglages « duaux » (logique ET affichage/filtre).** Une clé peut être à la fois un item/FilterSetting
  (page World/Progressive/Shared) et une clé logique (`SETTING_KEYS`) — ex. `skipZelda`, `ganonBossKey`, les
  items shared/progressive. Elle reste sur sa page d'origine mais doit écrire **les 2 champs** : `card_row` /
  `bool_card_row` (settings_window.rs), quand `is_logic_key(key)` (= `data::SETTING_KEYS.contains`), miroitent
  la valeur dans `raw_settings` en plus de `values` ; `card_row` lit `raw_settings` pour l'affichage exact.
  Cas dérivé : **`fire_temple_open_as_child`** n'a pas de toggle — `apply()` le calcule depuis la présence du
  membre `fireChild` dans `openDungeonsOot` (set de la page Accès), avant `apply_oot` ; l'ancienne carte
  « Open Dungeons » de World Items a été retirée.
- **Settings « set » multi-membres (solveur).** `openDungeonsOot/Mm`, `ganonTrials` et `clearStateDungeonsMm`
  sont référencés par la logique via `setting(k, membre)` pour **plusieurs** membres. Le modèle `setting_value`
  (une valeur/clé) ne suffit pas : `logic/inputs.rs` tokenise la valeur brute (séparateurs `,`/espaces) dans
  `settings_multi`, et `setting_has(k, v)` (trait `Inputs`/`WorldState`, `Op::SettingEq`) teste l'appartenance.
  L'éditeur écrit les membres joints par `,` (vide → `none`). **Source spoiler = section `World Flags`** (pas
  `Settings`) : OoTMM y écrit ces sets par **nom d'affichage** (liste `- …` ou inline `all`/`none`) ;
  `settings.rs::parse_logic_sets` mappe ces labels → valeurs brutes (tables `OPEN_DUNGEONS_OOT/MM`,
  `GANON_TRIALS` ; `clearStateDungeonsMm` réutilise `OPEN_DUNGEONS_MM` ; ⚠️ certains labels diffèrent du
  snapshot, ex. « Dodongo's Cavern » → `DC`).
