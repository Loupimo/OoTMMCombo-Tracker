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


#pragma region ProgressionTab

namespace {

// Static lookup: known display name -> matching icon path under ./Resources/Common/.
struct IconMapping
{
    const char* Keyword;
    const char* Path;
};

const IconMapping ICON_MAPPINGS[] = {
    { "kokiri sword",       "./Resources/Common/Sword.png" },
    { "master sword",       "./Resources/Common/Sword.png" },
    { "biggoron",           "./Resources/Common/Sword.png" },
    { "fairy bow",          "./Resources/Common/Bow.png" },
    { "fairy slingshot",    "./Resources/Common/Slingshot.png" },
    { "slingshot",          "./Resources/Common/Slingshot.png" },
    { "boomerang",          "./Resources/Common/Boomerang.png" },
    { "hookshot",           "./Resources/Common/Hookshot.png" },
    { "longshot",           "./Resources/Common/Hookshot.png" },
    { "megaton hammer",     "./Resources/Common/Hammer.png" },
    { "lens of truth",      "./Resources/Common/Items/Lens.png" },
    { "magic beans",        "./Resources/Common/Items/Bean.png" },
    { "fire arrow",         "./Resources/Common/Items/Fire.png" },
    { "ice arrow",          "./Resources/Common/Items/Ice.png" },
    { "light arrow",        "./Resources/Common/Items/Light.png" },
    { "fairy ocarina",      "./Resources/Common/Ocarina.png" },
    { "ocarina of time",    "./Resources/Common/Ocarina.png" },
    { "ocarina",            "./Resources/Common/Ocarina.png" },
    { "empty bottle",       "./Resources/Common/Items/Bottle.png" },
    { "bottle",             "./Resources/Common/Items/Bottle.png" },
    { "bombchu",            "./Resources/Common/Items/Bombchu.png" },
    { "lon lon milk",       "./Resources/Common/Items/Milk.png" },
    { "red potion",         "./Resources/Common/Items/Red_Potion.png" },
    { "din's fire",         "./Resources/Common/Items/Fire.png" },
    { "farore's wind",      "./Resources/Common/Items/Light.png" },
    { "nayru's love",       "./Resources/Common/Items/Light.png" },
    { "kokiri's emerald",   "./Resources/Common/Quest/Defense.png" },
    { "goron's ruby",       "./Resources/Common/Quest/Defense.png" },
    { "zora's sapphire",    "./Resources/Common/Quest/Defense.png" },
    { "forest medallion",   "./Resources/Common/Quest/Defense.png" },
    { "fire medallion",     "./Resources/Common/Quest/Defense.png" },
    { "water medallion",    "./Resources/Common/Quest/Defense.png" },
    { "spirit medallion",   "./Resources/Common/Quest/Defense.png" },
    { "shadow medallion",   "./Resources/Common/Quest/Defense.png" },
    { "light medallion",    "./Resources/Common/Quest/Defense.png" },
    { "magic upgrade",      "./Resources/Common/Quest/Magic.png" },
    { "double magic",       "./Resources/Common/Quest/Double_Magic.png" },
    { "double defense",     "./Resources/Common/Quest/Defense.png" },
    { "small key",          "./Resources/Common/Small_Key.png" },
    { "boss key",           "./Resources/Common/Boss_Key.png" },
    { "compass",            "./Resources/Common/Compass.png" },
    { "map ",               "./Resources/Common/Map.png" },
    { "gerudo's membership card", "./Resources/Common/NPC.png" },
    { "gold skulltula",     "./Resources/Common/Gold_Skulltula.png" },
    { "stray fairy",        "./Resources/Common/Stray_Fairy.png" },
    { "fairy",              "./Resources/Common/Fairy.png" },
    { "stone of agony",     "./Resources/Common/NPC.png" },
    { "triforce",           "./Resources/Common/Quest/Defense.png" },
    { "rupee",              "./Resources/Common/Rupee.png" },
    { "deku shield",        "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "hylian shield",      "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "mirror shield",      "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "goron tunic",        "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "zora tunic",         "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "iron boots",         "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "hover boots",        "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "silver gauntlets",   "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "golden gauntlets",   "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "silver scale",       "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "golden scale",       "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "bomb bag",           "./Resources/Common/Equipment/Bomb_Bag.png" },
    { "big bomb bag",       "./Resources/Common/Equipment/Big_Bomb_Bag.png" },
    { "biggest bomb bag",   "./Resources/Common/Equipment/Biggest_Bomb_Bag.png" },
    { "quiver",             "./Resources/Common/Equipment/Big_Quiver.png" },
    { "wallet",             "./Resources/Common/Equipment/Wallet.png" },
    { "lullaby",            "./Resources/Common/Song.png" },
    { "epona",              "./Resources/Common/Song.png" },
    { "saria",              "./Resources/Common/Song.png" },
    { "sun's song",         "./Resources/Common/Song.png" },
    { "song of time",       "./Resources/Common/Song.png" },
    { "song of storms",     "./Resources/Common/Song.png" },
    { "song of healing",    "./Resources/Common/Song.png" },
    { "song of soaring",    "./Resources/Common/Song.png" },
    { "elegy of emptiness", "./Resources/Common/Song.png" },
    { "sonata of awakening","./Resources/Common/Song.png" },
    { "goron lullaby",      "./Resources/Common/Song.png" },
    { "oath to order",      "./Resources/Common/Song.png" },
    { "deku mask",          "./Resources/Common/Mask.png" },
    { "goron mask",         "./Resources/Common/Mask.png" },
    { "zora mask",          "./Resources/Common/Mask.png" },
    { "fierce deity",       "./Resources/Common/Mask.png" },
    { "bunny hood",         "./Resources/Common/Masks/Bunny.png" },
    { "keaton mask",        "./Resources/Common/Masks/Keaton.png" },
    { "mask of truth",      "./Resources/Common/Masks/Truth.png" },
    { "mask",               "./Resources/Common/Mask.png" },
    { "remains",            "./Resources/Common/Boss_Key.png" },
    { "owl statue",         "./Resources/Common/Owl.png" },
    { "owl",                "./Resources/Common/Owl.png" },
    { "soul",               "./Resources/Common/Stray_Fairy.png" },
    { "heart container",    "./Resources/Common/HC.png" },
    { "piece of heart",     "./Resources/Common/HP.png" },
    { "recovery heart",     "./Resources/Common/Heart.png" },
};

const size_t NUM_ICON_MAPPINGS = sizeof(ICON_MAPPINGS) / sizeof(ICON_MAPPINGS[0]);

} // namespace


QString ProgressionTab::ResolveIconPath(const QString& Name)
{
    QString lower = Name.toLower();
    for (size_t i = 0; i < NUM_ICON_MAPPINGS; ++i)
    {
        if (lower.contains(QString::fromUtf8(ICON_MAPPINGS[i].Keyword)))
        {
            return QString::fromUtf8(ICON_MAPPINGS[i].Path);
        }
    }
    return QString("./Resources/Common/Grass.png");
}


QString ProgressionTab::NormalizeItemName(const QString& Name)
{
    QString cleaned = Name;
    cleaned.replace(QRegularExpression("\\s*\\((OoT|MM)\\)\\s*"), "");
    cleaned = cleaned.trimmed().toLower();
    return cleaned;
}


ProgressionTab::ProgressionTab(OoTMMComboTracker* Owner, QWidget* Parent)
    : QWidget(Parent), WinOwner(Owner)
{
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


QWidget* ProgressionTab::BuildPage(const QList<QPair<QString, QList<QPair<QString, QString>>>>& Sections,
                                   QHash<QString, ItemIconWidget*>& TargetMap, int Game)
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(14);

    for (const auto& sectionPair : Sections)
    {
        // Section header.
        QLabel* header = new QLabel(sectionPair.first, content);
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

        for (const auto& itemPair : sectionPair.second)
        {
            const QString& displayName = itemPair.first;
            const QString& flagsOrIcon = itemPair.second; // "counter" => stackable, otherwise icon override or "".

            bool isCounter = (flagsOrIcon == "counter");
            QString iconPath = (flagsOrIcon.isEmpty() || isCounter) ? ProgressionTab::ResolveIconPath(displayName) : flagsOrIcon;

            ItemIconWidget* widget = new ItemIconWidget(iconPath, displayName, isCounter, gridHost);
            widget->Game = Game;
            grid->addWidget(widget, row, col);

            connect(widget, &ItemIconWidget::Selected, this, &ProgressionTab::ShowDetailFor);

            // Register the lookup key (normalized base name) -> widget.
            TargetMap.insert(ProgressionTab::NormalizeItemName(displayName), widget);

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


QWidget* ProgressionTab::BuildSoulsPage()
{
    // The souls page reuses the same grid layout — the data comes from a flat list of soul names
    // grouped by category. Souls map to OOT_GAME by convention here so the detail panel can resolve
    // their scene name when needed.
    QList<QPair<QString, QList<QPair<QString, QString>>>> sections;

    sections.append({ "OoT Boss Souls", {
        { "Soul of Gohma", "" }, { "Soul of King Dodongo", "" }, { "Soul of Barinade", "" },
        { "Soul of Phantom Ganon", "" }, { "Soul of Volvagia", "" }, { "Soul of Morpha", "" },
        { "Soul of Bongo Bongo", "" }, { "Soul of Twinrova", "" }, { "Soul of Ganondorf", "" }
    }});

    sections.append({ "MM Boss Souls", {
        { "Soul of Odolwa", "" }, { "Soul of Goht", "" }, { "Soul of Gyorg", "" }, { "Soul of Twinmold", "" }
    }});

    sections.append({ "OoT Enemy Souls", {
        { "Soul of Stalfos", "" }, { "Soul of Octoroks", "" }, { "Soul of Wallmasters", "" },
        { "Soul of Dodongos", "" }, { "Soul of Keese", "" }, { "Soul of Tektites", "" },
        { "Soul of Leevers", "" }, { "Soul of Peahats", "" }, { "Soul of Skulltulas", "" },
        { "Soul of Moblins", "" }, { "Soul of Armos", "" }, { "Soul of Deku Babas", "" },
        { "Soul of Deku Scrubs", "" }, { "Soul of Bubbles", "" }, { "Soul of Beamos", "" },
        { "Soul of Floormasters", "" }, { "Soul of ReDeads/Gibdos", "" },
        { "Soul of Skullwalltulas", "" }, { "Soul of Iron Knuckles", "" },
        { "Soul of Skull Kids", "" }, { "Soul of Flying Pots", "" },
        { "Soul of Freezards", "" }, { "Soul of Wolfoses", "" }, { "Soul of Stalchildren", "" },
        { "Soul of Like Likes", "" }, { "Soul of Shell Blades", "" }
    }});

    return this->BuildPage(sections, this->SoulsItemMap, OOT_GAME);
}


void ProgressionTab::BuildPages()
{
    // ---- OoT page ----------------------------------------------------------
    QList<QPair<QString, QList<QPair<QString, QString>>>> ootSections;

    ootSections.append({ "Weapons & Tools", {
        { "Kokiri Sword", "" }, { "Master Sword", "" }, { "Biggoron's Sword", "" },
        { "Fairy Bow", "" }, { "Fire Arrows", "" }, { "Ice Arrows", "" }, { "Light Arrows", "" },
        { "Fairy Slingshot", "" }, { "Boomerang", "" }, { "Progressive Hookshot", "" },
        { "Megaton Hammer", "" }, { "Bombchu", "counter" }, { "Lens of Truth", "" }, { "Magic Beans", "" },
        { "Fairy Ocarina", "" }, { "Ocarina of Time", "" }, { "Empty Bottle", "counter" },
        { "Din's Fire", "" }, { "Farore's Wind", "" }, { "Nayru's Love", "" }
    }});

    ootSections.append({ "Equipment", {
        { "Deku Shield", "" }, { "Hylian Shield", "" }, { "Mirror Shield", "" },
        { "Goron Tunic", "" }, { "Zora Tunic", "" }, { "Iron Boots", "" }, { "Hover Boots", "" },
        { "Silver Gauntlets", "" }, { "Golden Gauntlets", "" }, { "Silver Scale", "" }, { "Golden Scale", "" },
        { "Bomb Bag", "" }, { "Big Bomb Bag", "" }, { "Biggest Bomb Bag", "" },
        { "Big Quiver", "" }, { "Biggest Quiver", "" },
        { "Magic Upgrade", "" }, { "Double Magic", "" }, { "Double Defense", "" }
    }});

    ootSections.append({ "Stones & Medallions", {
        { "Kokiri's Emerald", "" }, { "Goron's Ruby", "" }, { "Zora's Sapphire", "" },
        { "Forest Medallion", "" }, { "Fire Medallion", "" }, { "Water Medallion", "" },
        { "Spirit Medallion", "" }, { "Shadow Medallion", "" }, { "Light Medallion", "" }
    }});

    ootSections.append({ "Songs", {
        { "Zelda's Lullaby", "" }, { "Epona's Song", "" }, { "Saria's Song", "" }, { "Sun's Song", "" },
        { "Song of Time", "" }, { "Song of Storms", "" },
        { "Minuet of Forest", "" }, { "Bolero of Fire", "" }, { "Serenade of Water", "" },
        { "Requiem of Spirit", "" }, { "Nocturne of Shadow", "" }, { "Prelude of Light", "" }
    }});

    ootSections.append({ "Counters", {
        { "Gold Skulltula Token", "counter" }, { "Triforce Piece", "counter" },
        { "Small Key", "counter" }, { "Boss Key", "counter" },
        { "Green Rupee", "counter" }, { "Blue Rupee", "counter" }, { "Red Rupee", "counter" },
        { "Purple Rupee", "counter" }, { "Gold Rupee", "counter" }
    }});

    QWidget* ootPage = this->BuildPage(ootSections, this->OoTItemMap, OOT_GAME);
    this->PageStack->addWidget(ootPage);

    // ---- MM page -----------------------------------------------------------
    QList<QPair<QString, QList<QPair<QString, QString>>>> mmSections;

    mmSections.append({ "Transformation Masks", {
        { "Deku Mask", "" }, { "Goron Mask", "" }, { "Zora Mask", "" }, { "Fierce Deity's Mask", "" }
    }});

    mmSections.append({ "Masks", {
        { "Bunny Hood", "" }, { "Keaton Mask", "" }, { "Mask of Truth", "" },
        { "Postman's Hat", "" }, { "All-Night Mask", "" }, { "Blast Mask", "" },
        { "Stone Mask", "" }, { "Great Fairy's Mask", "" }, { "Bremen Mask", "" },
        { "Don Gero's Mask", "" }, { "Kamaro's Mask", "" }, { "Romani's Mask", "" },
        { "Circus Leader's Mask", "" }, { "Kafei's Mask", "" }, { "Couple's Mask", "" },
        { "Mask of Scents", "" }, { "Garo's Mask", "" }, { "Captain's Hat", "" },
        { "Gibdo Mask", "" }, { "Giant's Mask", "" }
    }});

    mmSections.append({ "Boss Remains", {
        { "Odolwa's Remains", "" }, { "Goht's Remains", "" }, { "Gyorg's Remains", "" }, { "Twinmold's Remains", "" }
    }});

    mmSections.append({ "Weapons & Tools", {
        { "Hero's Bow", "" }, { "Hookshot", "" }, { "Lens of Truth", "" },
        { "Pictograph Box", "" }, { "Powder Keg", "" }, { "Great Fairy's Sword", "" },
        { "Bombchu", "counter" }, { "Magic Upgrade", "" }, { "Double Magic", "" }
    }});

    mmSections.append({ "Songs", {
        { "Song of Time", "" }, { "Song of Healing", "" }, { "Epona's Song", "" }, { "Song of Soaring", "" },
        { "Song of Storms", "" }, { "Sun's Song", "" }, { "Sonata of Awakening", "" },
        { "Goron Lullaby", "" }, { "New Wave Bossa Nova", "" }, { "Elegy of Emptiness", "" },
        { "Oath to Order", "" }
    }});

    mmSections.append({ "Counters", {
        { "Stray Fairy (Woodfall Temple)", "counter" }, { "Stray Fairy (Snowhead Temple)", "counter" },
        { "Stray Fairy (Great Bay Temple)", "counter" }, { "Stray Fairy (Stone Tower Temple)", "counter" },
        { "Stray Fairy (Clock Town)", "counter" },
        { "Swamp Skulltula Token", "counter" }, { "Ocean Skulltula Token", "counter" }
    }});

    QWidget* mmPage = this->BuildPage(mmSections, this->MMItemMap, MM_GAME);
    this->PageStack->addWidget(mmPage);

    // ---- Souls page --------------------------------------------------------
    this->PageStack->addWidget(this->BuildSoulsPage());
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

    QString key = ProgressionTab::NormalizeItemName(QString::fromUtf8(Item->ItemName));

    // Try the per-game maps first, then the souls map (souls span both games).
    ItemIconWidget* widget = nullptr;
    if (Game == OOT_GAME)
    {
        widget = this->OoTItemMap.value(key, nullptr);
    }
    else if (Game == MM_GAME)
    {
        widget = this->MMItemMap.value(key, nullptr);
    }
    if (widget == nullptr)
    {
        widget = this->SoulsItemMap.value(key, nullptr);
    }

    if (widget != nullptr)
    {
        widget->MarkFound(Object);
    }
}


void ProgressionTab::ResetProgress()
{
    for (ItemIconWidget* w : this->OoTItemMap.values())  if (w) w->ResetFound();
    for (ItemIconWidget* w : this->MMItemMap.values())   if (w) w->ResetFound();
    for (ItemIconWidget* w : this->SoulsItemMap.values())if (w) w->ResetFound();
}

#pragma endregion
