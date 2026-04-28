#include "UI/ProgressionTab.h"
#include "UI/OoTMMComboTracker.h"
#include "Combo/Scenes.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QPixmap>
#include <QImage>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QFrame>
#include <QScrollArea>
#include <QStringList>
#include <QRegularExpression>


#pragma region ItemIconWidget

ItemIconWidget::ItemIconWidget(const QString& IconPath, const QString& DisplayName, bool IsCounter, QWidget* Parent)
    : QWidget(Parent), DisplayName(DisplayName), IconPath(IconPath), IsCounter(IsCounter)
{
    this->setCursor(Qt::PointingHandCursor);
    this->setFixedSize(72, 92);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("background: transparent;");

    this->IconLabel = new QLabel(this);
    this->IconLabel->setGeometry(4, 0, 64, 64);
    this->IconLabel->setAlignment(Qt::AlignCenter);
    this->IconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->NameLabel = new QLabel(DisplayName, this);
    this->NameLabel->setGeometry(0, 64, 72, 28);
    this->NameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    this->NameLabel->setWordWrap(true);
    this->NameLabel->setStyleSheet("background: transparent; color: #7a9abf; font-size: 9px;");
    this->NameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->CountBadge = new QLabel(this);
    this->CountBadge->setGeometry(44, 44, 24, 18);
    this->CountBadge->setAlignment(Qt::AlignCenter);
    this->CountBadge->setStyleSheet(
        "background-color: #4a9edb;"
        "color: #ddeeff;"
        "border-radius: 6px;"
        "font-size: 9px;"
        "font-weight: 700;"
        "padding: 1px 3px;");
    this->CountBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->CountBadge->hide();

    this->RefreshVisual();
}


void ItemIconWidget::MarkFound(const ObjectInfo* Object)
{
    this->Found = true;
    if (this->IsCounter)
    {
        this->Count += 1;
    }

    if (Object != nullptr)
    {
        this->LastObject = Object;
        const char* sceneName = GetSceneName(this->Game, Object->Scene);
        QString sceneStr = sceneName ? QString::fromUtf8(sceneName) : QString::number(Object->Scene);
        if (!this->LocationsFound.contains(sceneStr))
        {
            this->LocationsFound.append(sceneStr);
        }
    }

    this->RefreshVisual();
}


void ItemIconWidget::ResetFound()
{
    this->Found = false;
    this->Count = 0;
    this->LocationsFound.clear();
    this->LastObject = nullptr;
    this->RefreshVisual();
}


void ItemIconWidget::mousePressEvent(QMouseEvent* Event)
{
    if (Event->button() == Qt::LeftButton)
    {
        emit this->Selected(this);
    }
    QWidget::mousePressEvent(Event);
}


void ItemIconWidget::resizeEvent(QResizeEvent* Event)
{
    QWidget::resizeEvent(Event);
}


void ItemIconWidget::RefreshVisual()
{
    QString resolvedPath = QFile::exists(this->IconPath) ? this->IconPath : QString("./Resources/Common/Grass.png");
    QPixmap pixmap(resolvedPath);
    if (pixmap.isNull())
    {
        pixmap = QPixmap("./Resources/Common/Grass.png");
    }
    pixmap = pixmap.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!this->Found)
    {   // Greyscale + dim alpha so the icon reads as a placeholder.

        QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < img.height(); ++y)
        {
            QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x)
            {
                QColor c = QColor::fromRgba(line[x]);
                int gray = qGray(c.rgb());
                line[x] = qRgba(gray, gray, gray, c.alpha() / 2);
            }
        }
        pixmap = QPixmap::fromImage(img);

        this->IconLabel->setGraphicsEffect(nullptr);
        this->NameLabel->setStyleSheet("background: transparent; color: #4a6a9a; font-size: 9px;");
    }
    else
    {   // Color icon with a soft glow so it pops out of the grid.

        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(this);
        glow->setColor(QColor(74, 158, 219, 200));
        glow->setBlurRadius(12);
        glow->setOffset(0, 0);
        this->IconLabel->setGraphicsEffect(glow);

        this->NameLabel->setStyleSheet("background: transparent; color: #ddeeff; font-size: 9px; font-weight: 600;");
    }

    this->IconLabel->setPixmap(pixmap);

    if (this->IsCounter && this->Count > 0)
    {
        this->CountBadge->setText(QString::number(this->Count));
        this->CountBadge->show();
    }
    else
    {
        this->CountBadge->hide();
    }

    QString tooltip = this->DisplayName;
    if (!this->LocationsFound.isEmpty())
    {
        tooltip += "\nFound in: " + this->LocationsFound.join(", ");
    }
    this->setToolTip(tooltip);
}

#pragma endregion


#pragma region Section data

namespace {

// Helper to compute the static count of an entries array at compile-time.
#define MAKE_SECTION(Title, Arr) { Title, Arr, sizeof(Arr) / sizeof(Arr[0]) }


//-------------------------- OoT page --------------------------//

const ProgEntry OoTWeaponsTools[] = {
    { EGameIcon::kokiri,        "Kokiri Sword",         "kokiri sword",     false },
    { EGameIcon::master,        "Master Sword",         "master sword",     false },
    { EGameIcon::biggoron,      "Biggoron's Sword",     "biggoron",         false },
    { EGameIcon::bow_oot,       "Fairy Bow",            "fairy bow",        false },
    { EGameIcon::fire_arrow,    "Fire Arrows",          "fire arrow",       false },
    { EGameIcon::ice_arrow,     "Ice Arrows",           "ice arrow",        false },
    { EGameIcon::light_arrow,   "Light Arrows",         "light arrow",      false },
    { EGameIcon::slingshot,     "Fairy Slingshot",      "slingshot",        false },
    { EGameIcon::boomrang,      "Boomerang",            "boomerang",        false },
    { EGameIcon::hookshot_oot,  "Progressive Hookshot", "hookshot",         false },
    { EGameIcon::hammer,        "Megaton Hammer",       "hammer",           false },
    { EGameIcon::bombchu,       "Bombchu",              "bombchu",          true  },
    { EGameIcon::lens,          "Lens of Truth",        "lens",             false },
    { EGameIcon::bean,          "Magic Beans",          "magic bean",       false },
    { EGameIcon::fairy_ocarina, "Fairy Ocarina",        "fairy ocarina",    false },
    { EGameIcon::ocarina,       "Ocarina of Time",      "ocarina of time",  false },
    { EGameIcon::bottle,        "Empty Bottle",         "empty bottle",     true  },
    { EGameIcon::din,           "Din's Fire",           "din",              false },
    { EGameIcon::farore,        "Farore's Wind",        "farore",           false },
    { EGameIcon::nayru,         "Nayru's Love",         "nayru",            false },
};

const ProgEntry OoTEquipment[] = {
    { EGameIcon::deku_shield,        "Deku Shield",      "deku shield",      false },
    { EGameIcon::hylian_shield,      "Hylian Shield",    "hylian shield",    false },
    { EGameIcon::mirror_oot,         "Mirror Shield",    "mirror shield",    false },
    { EGameIcon::goron_tunic,        "Goron Tunic",      "goron tunic",      false },
    { EGameIcon::zora_tunic,         "Zora Tunic",       "zora tunic",       false },
    { EGameIcon::iron,               "Iron Boots",       "iron boots",       false },
    { EGameIcon::hover,              "Hover Boots",      "hover boots",      false },
    { EGameIcon::silver_gauntlet,    "Silver Gauntlets", "silver gauntlet",  false },
    { EGameIcon::golden_gauntlet,    "Golden Gauntlets", "golden gauntlet",  false },
    { EGameIcon::silver,             "Silver Scale",     "silver scale",     false },
    { EGameIcon::golden,             "Golden Scale",     "golden scale",     false },
    { EGameIcon::bomb_bag,           "Bomb Bag",         "bomb bag",         false },
    { EGameIcon::big_bomb,           "Big Bomb Bag",     "big bomb",         false },
    { EGameIcon::biggest_bomb,       "Biggest Bomb Bag", "biggest bomb",     false },
    { EGameIcon::big_quiver,         "Big Quiver",       "big quiver",       false },
    { EGameIcon::biggest_quiver,     "Biggest Quiver",   "biggest quiver",   false },
    { EGameIcon::magic_upgrade,      "Magic Upgrade",    "magic upgrade",    false },
    { EGameIcon::large_magic_upgrade,"Double Magic",     "larger magic",     false },
    { EGameIcon::defense,            "Double Defense",   "double defense",   false },
};

const ProgEntry OoTStonesMedallions[] = {
    { EGameIcon::emerald,   "Kokiri's Emerald",  "emerald",          false },
    { EGameIcon::ruby,      "Goron's Ruby",      "ruby",             false },
    { EGameIcon::sapphire,  "Zora's Sapphire",   "sapphire",         false },
    { EGameIcon::forest,    "Forest Medallion",  "forest medallion", false },
    { EGameIcon::fire,      "Fire Medallion",    "fire medallion",   false },
    { EGameIcon::water,     "Water Medallion",   "water medallion",  false },
    { EGameIcon::spirit,    "Spirit Medallion",  "spirit medallion", false },
    { EGameIcon::shadow,    "Shadow Medallion",  "shadow medallion", false },
    { EGameIcon::light,     "Light Medallion",   "light medallion",  false },
};

const ProgEntry OoTSongs[] = {
    // Ocarina songs share EGameIcon::song -> disambiguated by LookupKey.
    { EGameIcon::song,        "Zelda's Lullaby",     "lullaby",         false },
    { EGameIcon::song,        "Epona's Song",        "epona",           false },
    { EGameIcon::song,        "Saria's Song",        "saria",           false },
    { EGameIcon::song,        "Sun's Song",          "sun's song",      false },
    { EGameIcon::song,        "Song of Time",        "song of time",    false },
    { EGameIcon::song,        "Song of Storms",      "song of storms",  false },
    // Warp songs each have a unique EGameIcon, but "Note from X" items share it.
    { EGameIcon::song_green,  "Minuet of Forest",    "minuet",          false },
    { EGameIcon::song_red,    "Bolero of Fire",      "bolero",          false },
    { EGameIcon::song_blue,   "Serenade of Water",   "serenade",        false },
    { EGameIcon::song_orange, "Requiem of Spirit",   "requiem",         false },
    { EGameIcon::song_purple, "Nocturne of Shadow",  "nocturne",        false },
    { EGameIcon::song_yellow, "Prelude of Light",    "prelude",         false },
};

const ProgEntry OoTCounters[] = {
    { EGameIcon::gs_token,       "Gold Skulltula Token", "gold skulltula", true },
    { EGameIcon::triforce_piece, "Triforce Piece",       "triforce piece", true },
    { EGameIcon::small_key,      "Small Key",            "small key",      true },
    { EGameIcon::boss_key,       "Boss Key",             "boss key",       true },
    { EGameIcon::rupee,          "Green Rupee",          "green rupee",    true },
    { EGameIcon::blue_rupee,     "Blue Rupee",           "blue rupee",     true },
    { EGameIcon::red_rupee,      "Red Rupee",            "red rupee",      true },
    { EGameIcon::purple_rupee,   "Purple Rupee",         "purple rupee",   true },
    { EGameIcon::gold_rupee,     "Gold Rupee",           "gold rupee",     true },
};

const ProgSection OoTSections[] = {
    MAKE_SECTION("Weapons & Tools",     OoTWeaponsTools),
    MAKE_SECTION("Equipment",           OoTEquipment),
    MAKE_SECTION("Stones & Medallions", OoTStonesMedallions),
    MAKE_SECTION("Songs",               OoTSongs),
    MAKE_SECTION("Counters",            OoTCounters),
};


//-------------------------- MM page --------------------------//

const ProgEntry MMTransformationMasks[] = {
    { EGameIcon::deku,   "Deku Mask",            "deku mask",          false },
    { EGameIcon::goron,  "Goron Mask",           "goron mask",         false },
    { EGameIcon::zora,   "Zora Mask",            "zora mask",          false },
    { EGameIcon::deity,  "Fierce Deity's Mask",  "fierce deity",       false },
};

const ProgEntry MMMasks[] = {
    { EGameIcon::bunny,      "Bunny Hood",            "bunny hood",          false },
    { EGameIcon::keaton,     "Keaton Mask",           "keaton",              false },
    { EGameIcon::truth,      "Mask of Truth",         "mask of truth",       false },
    { EGameIcon::postman,    "Postman's Hat",         "postman",             false },
    { EGameIcon::night,      "All-Night Mask",        "all-night",           false },
    { EGameIcon::blast,      "Blast Mask",            "blast mask",          false },
    { EGameIcon::stone,      "Stone Mask",            "stone mask",          false },
    { EGameIcon::fairy_mask, "Great Fairy's Mask",    "great fairy's mask",  false },
    { EGameIcon::bremen,     "Bremen Mask",           "bremen",              false },
    { EGameIcon::gero,       "Don Gero's Mask",       "don gero",            false },
    { EGameIcon::kamaro,     "Kamaro's Mask",         "kamaro",              false },
    { EGameIcon::romani,     "Romani's Mask",         "romani's mask",       false },
    { EGameIcon::troupe,     "Circus Leader's Mask",  "circus leader",       false },
    { EGameIcon::kafei,      "Kafei's Mask",          "kafei",               false },
    { EGameIcon::couple,     "Couple's Mask",         "couple",              false },
    { EGameIcon::scents,     "Mask of Scents",        "mask of scents",      false },
    { EGameIcon::garo,       "Garo's Mask",           "garo",                false },
    { EGameIcon::captain,    "Captain's Hat",         "captain",             false },
    { EGameIcon::gibdo,      "Gibdo Mask",            "gibdo",               false },
    { EGameIcon::giant,      "Giant's Mask",          "giant's mask",        false },
};

const ProgEntry MMRemains[] = {
    { EGameIcon::odolwa,    "Odolwa's Remains",   "odolwa",   false },
    { EGameIcon::goht,      "Goht's Remains",     "goht",     false },
    { EGameIcon::gyorg,     "Gyorg's Remains",    "gyorg",    false },
    { EGameIcon::twinmold,  "Twinmold's Remains", "twinmold", false },
};

const ProgEntry MMWeaponsTools[] = {
    { EGameIcon::bow_mm,            "Hero's Bow",          "hero's bow",         false },
    { EGameIcon::hookshot_mm,       "Hookshot",            "hookshot",           false },
    { EGameIcon::lens,              "Lens of Truth",       "lens",               false },
    { EGameIcon::picto,             "Pictograph Box",      "pictograph",         false },
    { EGameIcon::powder,            "Powder Keg",          "powder keg",         false },
    { EGameIcon::fairy_sword,       "Great Fairy's Sword", "great fairy's sword",false },
    { EGameIcon::bombchu,           "Bombchu",             "bombchu",            true  },
    { EGameIcon::magic_upgrade,     "Magic Upgrade",       "magic upgrade",      false },
    { EGameIcon::large_magic_upgrade,"Double Magic",       "larger magic",       false },
};

const ProgEntry MMSongs[] = {
    // Ocarina songs share EGameIcon::song -> disambiguated by LookupKey.
    { EGameIcon::song,        "Song of Time",        "song of time",    false },
    { EGameIcon::song,        "Song of Healing",     "song of healing", false },
    { EGameIcon::song,        "Epona's Song",        "epona",           false },
    { EGameIcon::song,        "Song of Soaring",     "song of soaring", false },
    { EGameIcon::song,        "Song of Storms",      "song of storms",  false },
    { EGameIcon::song,        "Sun's Song",          "sun's song",      false },
    // Warp / dungeon songs have unique EGameIcons but "Note from X" items share them.
    { EGameIcon::song_green,  "Sonata of Awakening", "sonata",          false },
    { EGameIcon::song_red,    "Goron Lullaby",       "goron lullaby",   false },
    { EGameIcon::song_blue,   "New Wave Bossa Nova", "bossa nova",      false },
    { EGameIcon::song_orange, "Elegy of Emptiness",  "elegy",           false },
    { EGameIcon::song_purple, "Oath to Order",       "oath to order",   false },
};

const ProgEntry MMCounters[] = {
    { EGameIcon::sf,              "Stray Fairy (Woodfall)",      "woodfall temple",     true },
    { EGameIcon::sf_green,        "Stray Fairy (Snowhead)",      "snowhead temple",     true },
    { EGameIcon::sf_blue,         "Stray Fairy (Great Bay)",     "great bay temple",    true },
    { EGameIcon::sf_yellow,       "Stray Fairy (Stone Tower)",     "stone tower temple",  true },
    { EGameIcon::sf_orange,       "Stray Fairy (Clock Town)",      "clock town",          true },
    { EGameIcon::swamp_token,     "Swamp Skulltula Token", "swamp skulltula",     true },
    { EGameIcon::ocean_token,     "Ocean Skulltula Token", "ocean skulltula",     true },
};

const ProgSection MMSections[] = {
    MAKE_SECTION("Transformation Masks", MMTransformationMasks),
    MAKE_SECTION("Masks",                MMMasks),
    MAKE_SECTION("Boss Remains",         MMRemains),
    MAKE_SECTION("Weapons & Tools",      MMWeaponsTools),
    MAKE_SECTION("Songs",                MMSongs),
    MAKE_SECTION("Counters",             MMCounters),
};


//-------------------------- Souls page --------------------------//

const ProgEntry SoulsOoTBoss[] = {
    { EGameIcon::soul_of_boss, "Soul of Gohma",         "queen gohma", false },
    { EGameIcon::soul_of_boss, "Soul of King Dodongo",  "king dodongo",false },
    { EGameIcon::soul_of_boss, "Soul of Barinade",      "barinade",    false },
    { EGameIcon::soul_of_boss, "Soul of Phantom Ganon", "phantom",     false },
    { EGameIcon::soul_of_boss, "Soul of Volvagia",      "volvagia",    false },
    { EGameIcon::soul_of_boss, "Soul of Morpha",        "morpha",      false },
    { EGameIcon::soul_of_boss, "Soul of Bongo Bongo",   "bongo",       false },
    { EGameIcon::soul_of_boss, "Soul of Twinrova",      "twinrova",    false },
    { EGameIcon::soul_of_boss, "Soul of Ganondorf",     "ganondorf",   false },
};

const ProgEntry SoulsMMBoss[] = {
    { EGameIcon::soul_of_boss, "Soul of Odolwa",   "odolwa",   false },
    { EGameIcon::soul_of_boss, "Soul of Goht",     "goht",     false },
    { EGameIcon::soul_of_boss, "Soul of Gyorg",    "gyorg",    false },
    { EGameIcon::soul_of_boss, "Soul of Twinmold", "twinmold", false },
};

const ProgEntry SoulsOoTEnemies[] = {
    { EGameIcon::soul_of_foe, "Soul of Stalfos",         "stalfos",        false },
    { EGameIcon::soul_of_foe, "Soul of Octoroks",        "octoroks",       false },
    { EGameIcon::soul_of_foe, "Soul of Wallmasters",     "wallmasters",    false },
    { EGameIcon::soul_of_foe, "Soul of Dodongos",        "dodongos",       false },
    { EGameIcon::soul_of_foe, "Soul of Keese",           "keese",          false },
    { EGameIcon::soul_of_foe, "Soul of Tektites",        "tektites",       false },
    { EGameIcon::soul_of_foe, "Soul of Leevers",         "leevers",        false },
    { EGameIcon::soul_of_foe, "Soul of Peahats",         "peahats",        false },
    { EGameIcon::soul_of_foe, "Soul of Skulltulas",      "skulltulas",     false },
    { EGameIcon::soul_of_foe, "Soul of Moblins",         "moblins",        false },
    { EGameIcon::soul_of_foe, "Soul of Armos",           "armos",          false },
    { EGameIcon::soul_of_foe, "Soul of Deku Babas",      "deku babas",     false },
    { EGameIcon::soul_of_foe, "Soul of Deku Scrubs",     "deku scrubs",    false },
    { EGameIcon::soul_of_foe, "Soul of Bubbles",         "bubbles",        false },
    { EGameIcon::soul_of_foe, "Soul of Beamos",          "beamos",         false },
    { EGameIcon::soul_of_foe, "Soul of Floormasters",    "floormasters",   false },
    { EGameIcon::soul_of_foe, "Soul of ReDeads/Gibdos",  "redeads",        false },
    { EGameIcon::soul_of_foe, "Soul of Skullwalltulas",  "skullwalltulas", false },
    { EGameIcon::soul_of_foe, "Soul of Iron Knuckles",   "iron knuckles",  false },
    { EGameIcon::soul_of_foe, "Soul of Skull Kids",      "skull kids",     false },
    { EGameIcon::soul_of_foe, "Soul of Flying Pots",     "flying pots",    false },
    { EGameIcon::soul_of_foe, "Soul of Freezards",       "freezards",      false },
    { EGameIcon::soul_of_foe, "Soul of Wolfoses",        "wolfoses",       false },
    { EGameIcon::soul_of_foe, "Soul of Stalchildren",    "stalchildren",   false },
    { EGameIcon::soul_of_foe, "Soul of Like Likes",      "like likes",     false },
    { EGameIcon::soul_of_foe, "Soul of Shell Blades",    "shell blades",   false },
};

const ProgSection SoulsSections[] = {
    MAKE_SECTION("OoT Boss Souls",   SoulsOoTBoss),
    MAKE_SECTION("MM Boss Souls",    SoulsMMBoss),
    MAKE_SECTION("OoT Enemy Souls",  SoulsOoTEnemies),
};

#undef MAKE_SECTION

} // namespace

#pragma endregion


#pragma region ProgressionTab


ProgressionTab* ProgressionTab::sInstance = nullptr;


ProgressionTab* ProgressionTab::GetInstance()
{
    return sInstance;
}


QString ProgressionTab::NormalizeItemName(const QString& Name)
{
    QString cleaned = Name;
    cleaned.replace(QRegularExpression("\\s*\\((OoT|MM)\\)\\s*"), "");
    cleaned = cleaned.trimmed().toLower();
    return cleaned;
}


ItemIconWidget* ProgressionTab::FindByIcon(const GameProgData& Data, EGameIcon Icon, const QString& Normalized)
{
    auto it = Data.ByIcon.constFind(Icon);
    if (it == Data.ByIcon.constEnd()) return nullptr;

    const QList<ItemIconWidget*>& candidates = it.value();
    for (ItemIconWidget* w : candidates)
    {   // Empty LookupKey means there is a single widget for this icon -> match unconditionally.

        if (w->LookupKey.isEmpty() || Normalized.contains(w->LookupKey))
        {
            return w;
        }
    }
    return nullptr;
}


ItemIconWidget* ProgressionTab::FindByLookupKey(const GameProgData& Data, const QString& Normalized)
{
    for (ItemIconWidget* w : Data.All)
    {   // Only widgets with a non-empty LookupKey are eligible for the name fallback.

        if (!w->LookupKey.isEmpty() && Normalized.contains(w->LookupKey))
        {
            return w;
        }
    }
    return nullptr;
}


ProgressionTab::ProgressionTab(OoTMMComboTracker* Owner, QWidget* Parent)
    : QWidget(Parent), WinOwner(Owner)
{
    sInstance = this;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top sub-tab selector.
    this->SubTabBar = new QTabBar(this);
    this->SubTabBar->addTab("Ocarina of Time");
    this->SubTabBar->addTab("Majora's Mask");
    this->SubTabBar->addTab("Souls");
    this->SubTabBar->setExpanding(false);

    // Body: stacked pages on the left, fixed detail panel on the right.
    QWidget* body = new QWidget(this);
    QHBoxLayout* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    this->PageStack = new QStackedWidget(body);
    bodyLayout->addWidget(this->PageStack, 1);

    QFrame* sep = new QFrame(body);
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #1a3050;");
    bodyLayout->addWidget(sep);

    QWidget* detail = this->BuildDetailPanel();
    bodyLayout->addWidget(detail);

    mainLayout->addWidget(this->SubTabBar);
    mainLayout->addWidget(body, 1);

    this->BuildPages();

    connect(this->SubTabBar, &QTabBar::currentChanged, this->PageStack, &QStackedWidget::setCurrentIndex);
}


ProgressionTab::~ProgressionTab()
{
    if (sInstance == this) sInstance = nullptr;
}


QWidget* ProgressionTab::BuildPage(const ProgSection* Sections, size_t SectionCount, GameProgData& Target, int Game)
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(14);

    for (size_t s = 0; s < SectionCount; ++s)
    {
        const ProgSection& section = Sections[s];

        // Section header.
        QLabel* header = new QLabel(QString::fromUtf8(section.Title), content);
        header->setStyleSheet(
            "color: #4a9edb; "
            "font-size: 12px; "
            "font-weight: 700; "
            "letter-spacing: 0.05em; "
            "border-bottom: 1px solid #1a3050; "
            "padding-bottom: 4px;");
        contentLayout->addWidget(header);

        QWidget* gridHost = new QWidget(content);
        QGridLayout* grid = new QGridLayout(gridHost);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setHorizontalSpacing(8);
        grid->setVerticalSpacing(8);

        const int columns = 8;
        int row = 0;
        int col = 0;

        for (size_t i = 0; i < section.Count; ++i)
        {
            const ProgEntry& entry = section.Entries[i];

            // Resolve the icon path from the central IconsMetaInfo table.
            QString iconPath = QString::fromUtf8(IconsMetaInfo[(uint8_t)entry.Icon].IconPath);
            QString displayName = QString::fromUtf8(entry.DisplayName);

            ItemIconWidget* widget = new ItemIconWidget(iconPath, displayName, entry.IsCounter, gridHost);
            widget->Game = Game;
            widget->Icon = entry.Icon;
            widget->LookupKey = entry.LookupKey ? QString::fromUtf8(entry.LookupKey) : QString();
            grid->addWidget(widget, row, col);

            connect(widget, &ItemIconWidget::Selected, this, &ProgressionTab::ShowDetailFor);

            // Index by EGameIcon for fast lookup; multiple widgets may share the same icon.
            Target.ByIcon[entry.Icon].append(widget);
            Target.All.append(widget);

            col++;
            if (col >= columns)
            {
                col = 0;
                row++;
            }
        }

        if (col > 0)
        {   // Pad the unfinished row so widgets keep a consistent left alignment.

            for (int c = col; c < columns; ++c)
            {
                grid->setColumnStretch(c, 1);
            }
        }

        contentLayout->addWidget(gridHost);
    }

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    return scroll;
}


void ProgressionTab::BuildPages()
{
    QWidget* ootPage = this->BuildPage(OoTSections, sizeof(OoTSections) / sizeof(OoTSections[0]),
                                       this->OoTData, OOT_GAME);
    this->PageStack->addWidget(ootPage);

    QWidget* mmPage = this->BuildPage(MMSections, sizeof(MMSections) / sizeof(MMSections[0]),
                                      this->MMData, MM_GAME);
    this->PageStack->addWidget(mmPage);

    // Souls span both games; the detail panel uses OOT_GAME by convention to resolve scene names.
    QWidget* soulsPage = this->BuildPage(SoulsSections, sizeof(SoulsSections) / sizeof(SoulsSections[0]),
                                         this->SoulsData, OOT_GAME);
    this->PageStack->addWidget(soulsPage);
}


QWidget* ProgressionTab::BuildDetailPanel()
{
    QWidget* panel = new QWidget(this);
    panel->setFixedWidth(260);

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    this->DetailIcon = new QLabel(panel);
    this->DetailIcon->setFixedSize(80, 80);
    this->DetailIcon->setAlignment(Qt::AlignCenter);
    this->DetailIcon->setStyleSheet("background: transparent;");

    this->DetailName = new QLabel("Select an item", panel);
    this->DetailName->setAlignment(Qt::AlignCenter);
    this->DetailName->setStyleSheet("background: transparent; color: #ddeeff; font-size: 14px; font-weight: 700;");
    this->DetailName->setWordWrap(true);

    this->DetailStatus = new QLabel(panel);
    this->DetailStatus->setAlignment(Qt::AlignCenter);
    this->DetailStatus->setStyleSheet("background: transparent; color: #7a9abf; font-size: 11px;");

    this->DetailLocation = new QLabel(panel);
    this->DetailLocation->setAlignment(Qt::AlignCenter);
    this->DetailLocation->setWordWrap(true);
    this->DetailLocation->setStyleSheet("background: transparent; color: #7a9abf; font-size: 11px;");

    this->DetailCount = new QLabel(panel);
    this->DetailCount->setAlignment(Qt::AlignCenter);
    this->DetailCount->setStyleSheet("background: transparent; color: #4a9edb; font-size: 22px; font-weight: 700;");

    layout->addWidget(this->DetailIcon, 0, Qt::AlignHCenter);
    layout->addWidget(this->DetailName);
    layout->addWidget(this->DetailStatus);
    layout->addWidget(this->DetailCount);
    layout->addWidget(this->DetailLocation);
    layout->addStretch(1);

    return panel;
}


void ProgressionTab::ShowDetailFor(ItemIconWidget* Widget)
{
    if (Widget == nullptr) return;

    QPixmap pixmap(Widget->IconPath);
    if (pixmap.isNull())
    {
        pixmap = QPixmap("./Resources/Common/Grass.png");
    }
    pixmap = pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    this->DetailIcon->setPixmap(pixmap);

    this->DetailName->setText(Widget->DisplayName);

    if (Widget->Found)
    {
        this->DetailStatus->setText("\xE2\x9C\x93 Found");
        this->DetailStatus->setStyleSheet("background: transparent; color: #65e09a; font-size: 11px; font-weight: 600;");
    }
    else
    {
        this->DetailStatus->setText("\xE2\x9C\x97 Not found");
        this->DetailStatus->setStyleSheet("background: transparent; color: #ff6969; font-size: 11px; font-weight: 600;");
    }

    if (Widget->IsCounter)
    {
        this->DetailCount->setText(QString::number(Widget->Count));
        this->DetailCount->show();
    }
    else
    {
        this->DetailCount->hide();
    }

    if (!Widget->LocationsFound.isEmpty())
    {
        this->DetailLocation->setText("Found in: " + Widget->LocationsFound.join(", "));
    }
    else if (Widget->LastObject != nullptr)
    {
        const char* sceneName = GetSceneName(Widget->Game, Widget->LastObject->Scene);
        this->DetailLocation->setText(QString("Last: %1").arg(sceneName ? sceneName : ""));
    }
    else
    {
        this->DetailLocation->setText("");
    }
}


void ProgressionTab::OnItemFound(int Game, ObjectInfo* Object, const ItemInfo* Item)
{
    if (Item == nullptr || Item->ItemName == nullptr) return;

    QString normalized = NormalizeItemName(QString::fromUtf8(Item->ItemName));
    GameProgData* primary = nullptr;
    if (Game == OOT_GAME)      primary = &this->OoTData;
    else if (Game == MM_GAME)  primary = &this->MMData;

    // 1. Primary lookup: per-game icon hash, disambiguated by LookupKey when needed.
    ItemIconWidget* widget = nullptr;
    if (primary != nullptr)
    {
        widget = FindByIcon(*primary, Item->RenderType, normalized);
    }

    // 2. Souls span both games -> they live in their own registry.
    if (widget == nullptr)
    {
        widget = FindByIcon(this->SoulsData, Item->RenderType, normalized);
    }

    // 3. Fallback: Item->RenderType cannot be matched (e.g. RenderType=none for progressive items).
    //    Scan the per-game flat list and match by LookupKey only.
    if (widget == nullptr && primary != nullptr)
    {
        widget = FindByLookupKey(*primary, normalized);
    }

    if (widget != nullptr)
    {
        widget->MarkFound(Object);
    }
}


void ProgressionTab::RebuildFromSceneObjects()
{
    // Replay every collected/forced object from the central SceneObjects arrays.
    // Counters are summed naturally because OnItemFound increments on each call.
    this->ResetProgress();

    static const struct { int Game; size_t Count; } Pages[] = {
        { OOT_GAME, OOT_NUM_SCENES },
        { MM_GAME,  MM_NUM_SCENES  },
    };

    for (const auto& page : Pages)
    {
        SceneObjects* scenes = GetGameSceneObjects(page.Game);
        if (scenes == nullptr) continue;

        for (size_t s = 0; s < page.Count; ++s)
        {
            SceneObjects& scene = scenes[s];
            for (size_t o = 0; o < scene.NumOfObjs; ++o)
            {
                ObjectInfo& obj = scene.Objects[o];
                if (obj.Status != ObjectState::Hidden && obj.Item != nullptr)
                {
                    this->OnItemFound(page.Game, &obj, obj.Item);
                }
            }
        }
    }
}


void ProgressionTab::ResetProgress()
{
    for (ItemIconWidget* w : this->OoTData.All)   if (w) w->ResetFound();
    for (ItemIconWidget* w : this->MMData.All)    if (w) w->ResetFound();
    for (ItemIconWidget* w : this->SoulsData.All) if (w) w->ResetFound();
}

#pragma endregion
