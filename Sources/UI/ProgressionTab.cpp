#include "UI/ProgressionTab.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/AppConfig.h"
#include "UI/ProgressionEntry.h"
#include "UI/Settings.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
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
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QPushButton>
#include <QVector>


#pragma region ItemIconWidget

ItemIconWidget::ItemIconWidget(EGameIcon IconValue, const QString& DisplayName, bool IsCounter, QWidget* Parent)
    : QWidget(Parent), DisplayName(DisplayName), Icon(IconValue), IsCounter(IsCounter)
{
    this->setCursor(Qt::PointingHandCursor);
    this->setFixedSize(72, 108);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("background: transparent;");

    this->IconLabel = new QLabel(this);
    this->IconLabel->setGeometry(4, 0, 64, 64);
    this->IconLabel->setAlignment(Qt::AlignCenter);
    this->IconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    // Allow up to 3 wrapped lines so long entries like "Compass (Snowhead Temple)"
    // are fully visible instead of being clipped by a too-short label.
    this->NameLabel = new QLabel(DisplayName, this);
    this->NameLabel->setGeometry(0, 64, 72, 44);
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

    // The pixmap and visual styling are set up by RefreshVisual the first time
    // showEvent fires. Loading every page's icons here would pay the disk cost
    // for the four pages up-front, even though only one is visible at a time.
}


void ItemIconWidget::MarkFound(int Game, const ObjectInfo* Object)
{
    this->Found = true;
    if (this->IsCounter)
    {
        this->Count += 1;
    }

    if (Object != nullptr)
    {
        const char* sceneName = GetSceneName(Game, Object->Scene);
        QString sceneStr = sceneName ? QString::fromUtf8(sceneName) : QString::number(Object->Scene);
        if (!this->LocationsFound.contains(sceneStr))
        {
            this->LocationsFound.append(sceneStr);
        }
    }

    this->RefreshVisual();
}


void ItemIconWidget::MarkNotFound(int Game, const ObjectInfo* Object)
{
    if (this->IsCounter)
    {
        this->Count -= 1;

        if (this->Count == 0) this->Found = false;
    }
    else
    {
        this->Found = false;
    }

    if (Object != nullptr)
    {
        const char* sceneName = GetSceneName(Game, Object->Scene);
        QString sceneStr = sceneName ? QString::fromUtf8(sceneName) : QString::number(Object->Scene);
        this->LocationsFound.removeOne(sceneStr);
    }

    this->RefreshVisual();
}


void ItemIconWidget::ResetFound()
{
    this->Found = false;
    this->Count = 0;
    this->LocationsFound.clear();
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


void ItemIconWidget::showEvent(QShowEvent* Event)
{
    QWidget::showEvent(Event);

    // First time the widget is mapped to the screen: pull the pixmap and
    // render the proper visual. State updates that happened earlier (for
    // example replayed collected items during spoiler-log loading) called
    // RefreshVisual but were short-circuited by the VisualLoaded gate, so
    // running it now is enough to catch up with the latest Found/Count.
    if (!this->VisualLoaded)
    {
        this->VisualLoaded = true;
        this->RefreshVisual();
    }
}


void ItemIconWidget::RefreshVisual()
{
    // Lazy: defer all paint work until the widget is first mapped. MarkFound
    // / MarkNotFound calls that arrive while the page is still hidden land
    // here too — they update the data fields and bail; showEvent will pick
    // up the latest state when the user navigates to the page.
    if (!this->VisualLoaded) return;

    // Pull the cached pixmap from the GameIcons singleton (loaded once per
    // EGameIcon, then handed out via Qt's implicit COW so every consumer
    // shares the same backing buffer).
    QPixmap pixmap;
    QPixmap* sharedPx = GameIcons::GetGamePixmap(this->Icon);
    if (sharedPx != nullptr) pixmap = *sharedPx;
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


//ItemIconWidget* ProgressionTab::FindByIcon(const GameProgData& Data, EGameIcon Icon, const QString& Normalized)
ItemIconWidget* ProgressionTab::FindByIcon(const GameProgData& Data, const ItemInfo* Item)
{
    auto it = Data.ByIcon.constFind(Item->RenderType);
    if (it == Data.ByIcon.constEnd()) return nullptr;

    const QList<ItemIconWidget*>& candidates = it.value();
    for (ItemIconWidget* w : candidates)
    {   // Empty LookupKey means there is a single widget for this icon -> match unconditionally.

        //if (w->LookupKey.isEmpty() || Normalized.contains(w->LookupKey))
        if (w->LookupKeys.contains(Item->ItemID))
        {
            return w;
        }
    }
    return nullptr;
}


//ItemIconWidget* ProgressionTab::FindByLookupKey(const GameProgData& Data, const QString& Normalized)
ItemIconWidget* ProgressionTab::FindByLookupKey(const GameProgData& Data, const ItemInfo* Item)
{
    for (ItemIconWidget* w : Data.All)
    {   // Only widgets with a non-empty LookupKey are eligible for the name fallback.

        //if (!w->LookupKey.isEmpty() && Normalized.contains(w->LookupKey))
        if (w->LookupKeys.contains(Item->ItemID))
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
    this->SubTabBar->addTab("Collectibles");
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


QWidget* ProgressionTab::BuildPage(const ProgSection* Sections, size_t SectionCount, GameProgData& Target, int Game, int PageIndex)
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

        GridSection sectionLayout;
        sectionLayout.Grid = grid;
        sectionLayout.Header = header;
        sectionLayout.GridHost = gridHost;
        sectionLayout.PageIndex = PageIndex;

        const int columns = 16;

        // Reserve all 16 columns up-front with both a minimum width matching the
        // ItemIconWidget fixed width and a uniform stretch. The minimum width
        // forces every column to actually allocate space (setColumnStretch alone
        // is not enough — Qt may still collapse trailing columns that hold no
        // widget); the stretch ensures the residual space is divided evenly so
        // icons line up identically whether the section is a single partial row
        // or several rows with a partial last one.
        for (int c = 0; c < columns; ++c)
        {
            grid->setColumnMinimumWidth(c, 72);
            grid->setColumnStretch(c, 1);
        }

        int row = 0;
        int col = 0;

        for (size_t i = 0; i < section.Count; ++i)
        {
            const ProgEntry& entry = section.Entries[i];

            // Resolve the icon path from the central IconsMetaInfo table.
            QString displayName = QString::fromUtf8(entry.DisplayName);

            ItemIconWidget* widget = new ItemIconWidget(entry.Icon, displayName, entry.IsCounter, gridHost);
            widget->Game = Game;
            //widget->LookupKey = entry.LookupKey ? QString::fromUtf8(entry.LookupKey) : QString();
            widget->LookupKeys = widget->LookupKeys.unite(entry.LookupKeys);
            grid->addWidget(widget, row, col);

            connect(widget, &ItemIconWidget::Selected, this, &ProgressionTab::ShowDetailFor);

            // Index by EGameIcon for fast lookup; multiple widgets may share the same icon.
            Target.ByIcon[entry.Icon].append(widget);
            Target.All.append(widget);
            sectionLayout.Widgets.append(widget);

            col++;
            if (col >= columns)
            {
                col = 0;
                row++;
            }
        }

        contentLayout->addWidget(gridHost);
        this->Sections.append(sectionLayout);
    }

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    return scroll;
}


void ProgressionTab::BuildPages()
{
    QWidget* ootPage = this->BuildPage(OoTSections, NumOoTSections, this->OoTData, OOT_GAME, 0);
    this->PageStack->addWidget(ootPage);

    QWidget* mmPage = this->BuildPage(MMSections, NumMMSections, this->MMData, MM_GAME, 1);
    this->PageStack->addWidget(mmPage);

    // Souls span both games; the detail panel uses OOT_GAME by convention to resolve scene names.
    QWidget* soulsPage = this->BuildPage(SoulsSections, NumSoulsSections, this->SoulsData, OOT_GAME, 2);
    this->PageStack->addWidget(soulsPage);

    QWidget* collectiblesPage = this->BuildPage(CollectiblesSections, NumCollectiblesSections, this->CollectiblesData, OOT_GAME, 3);
    this->PageStack->addWidget(collectiblesPage);
}


QWidget* ProgressionTab::BuildDetailPanel()
{
    QWidget* panel = new QWidget(this);
    panel->setFixedWidth(300);

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

    this->DetailCount = new QLabel(panel);
    this->DetailCount->setAlignment(Qt::AlignCenter);
    this->DetailCount->setStyleSheet("background: transparent; color: #4a9edb; font-size: 22px; font-weight: 700;");

    this->DetailLocationsHeader = new QLabel("Locations", panel);
    this->DetailLocationsHeader->setStyleSheet(
        "background: transparent; color: #4a9edb; font-size: 11px; font-weight: 700; "
        "letter-spacing: 0.05em; padding-bottom: 3px;");

    this->DetailExpandToggle = new QPushButton("Collapse All", panel);
    this->DetailExpandToggle->setCursor(Qt::PointingHandCursor);
    this->DetailExpandToggle->setStyleSheet(
        "QPushButton { background: transparent; color: #4a9edb; border: 1px solid #1a3050; "
        "border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: 600; }"
        "QPushButton:hover { background-color: rgba(74, 158, 219, 30); border-color: #4a9edb; }"
        "QPushButton:pressed { background-color: rgba(74, 158, 219, 60); }");
    this->DetailExpandToggle->hide();
    QObject::connect(this->DetailExpandToggle, &QPushButton::clicked,
                     this, &ProgressionTab::OnToggleLocationsExpansion);

    QWidget* locationsHeaderRow = new QWidget(panel);
    QHBoxLayout* locationsHeaderLayout = new QHBoxLayout(locationsHeaderRow);
    locationsHeaderLayout->setContentsMargins(0, 0, 0, 0);
    locationsHeaderLayout->setSpacing(8);
    locationsHeaderLayout->addWidget(this->DetailLocationsHeader, 1, Qt::AlignBottom);
    locationsHeaderLayout->addWidget(this->DetailExpandToggle, 0, Qt::AlignBottom);
    locationsHeaderRow->setStyleSheet("background: transparent; border-bottom: 1px solid #1a3050;");

    this->DetailLocations = new QTreeWidget(panel);
    this->DetailLocations->setHeaderHidden(true);
    this->DetailLocations->setColumnCount(1);
    this->DetailLocations->setRootIsDecorated(true);
    this->DetailLocations->setIndentation(14);
    this->DetailLocations->setUniformRowHeights(false);
    this->DetailLocations->setSelectionMode(QAbstractItemView::SingleSelection);
    this->DetailLocations->setStyleSheet(
        "QTreeWidget { background: transparent; border: none; color: #ddeeff; } "
        "QTreeWidget::item { padding: 2px 0px; } "
        "QTreeWidget::item:hover { background-color: rgba(74, 158, 219, 30); } "
        "QTreeWidget::item:selected { background-color: rgba(74, 158, 219, 80); color: #ddeeff; }");

    QObject::connect(this->DetailLocations, &QTreeWidget::itemClicked, this, &ProgressionTab::OnLocationClicked);

    layout->addWidget(this->DetailIcon, 0, Qt::AlignHCenter);
    layout->addWidget(this->DetailName);
    layout->addWidget(this->DetailStatus);
    layout->addWidget(this->DetailCount);
    layout->addWidget(locationsHeaderRow);
    layout->addWidget(this->DetailLocations, 1);

    return panel;
}


void ProgressionTab::OnToggleLocationsExpansion()
{
    if (this->DetailLocations == nullptr || this->DetailExpandToggle == nullptr) return;

    this->DetailLocationsExpanded = !this->DetailLocationsExpanded;

    if (this->DetailLocationsExpanded)
    {
        this->DetailLocations->expandAll();
        this->DetailExpandToggle->setText("Collapse All");
    }
    else
    {
        this->DetailLocations->collapseAll();
        this->DetailExpandToggle->setText("Expand All");
    }
}


void ProgressionTab::ShowDetailFor(ItemIconWidget* Widget)
{
    if (Widget == nullptr) return;

    this->CurrentDetailWidget = Widget;

    QPixmap pixmap;
    QPixmap* sharedPx = GameIcons::GetGamePixmap(Widget->Icon);
    if (sharedPx != nullptr) pixmap = *sharedPx;
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

    this->BuildLocationTree(Widget);
}


bool ProgressionTab::ItemMatchesWidget(const ItemIconWidget* Widget, int /*Game*/, const ItemInfo* Item)
{
    if (Widget == nullptr || Item == nullptr) return false;

    // The widget enumerates every game item ID it stands for (ProgEntry::LookupKeys),
    // so a single ItemID lookup is sufficient — no need to also test the icon. Items
    // with the same icon but a different ItemID (e.g. two distinct rupee tiers) end
    // up in their own widget. The Game parameter is intentionally ignored because
    // the location tree must list the item's locations across both games.
    return Widget->LookupKeys.contains(Item->ItemID);
}


namespace {

/* Lightweight node carrying everything OnLocationClicked needs to navigate. */
struct LocationLeafData
{
    int Game;
    ObjectInfo* Object;
};

constexpr int kLocationGameRole       = Qt::UserRole + 100;
constexpr int kLocationObjectRole     = Qt::UserRole + 101;
constexpr int kLocationCollectedRole  = Qt::UserRole + 102;

/* Compute the icon to display for an object leaf, mirroring ObjectItemTree's logic. */
QIcon ResolveObjectIcon(const ObjectInfo* Obj)
{
    if (Obj == nullptr) return QIcon();

    EGameIcon iconID = Obj->MapIcon;
    switch (Obj->MapIcon)
    {
        case EGameIcon::type:        iconID = (EGameIcon)Obj->Type;       break;
        case EGameIcon::render_type: iconID = (EGameIcon)Obj->RenderType; break;
        default: break;
    }

    QIcon* base = GameIcons::GetGameIcon(iconID);
    return base ? *base : QIcon();
}

/* Build a desaturated, low-opacity version of an object icon to mark uncollected
   leaves at a glance — italics alone do not stand out enough against the colored
   icons of collected entries. */
QIcon MakeFadedIcon(const QIcon& Source)
{
    const QSize size(20, 20);
    QPixmap pixmap = Source.pixmap(size);
    if (pixmap.isNull()) return Source;

    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y)
    {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x)
        {
            QColor c = QColor::fromRgba(line[x]);
            int gray = qGray(c.rgb());
            line[x] = qRgba(gray, gray, gray, c.alpha() / 3);
        }
    }
    return QIcon(QPixmap::fromImage(img));
}

} // namespace


void ProgressionTab::BuildLocationTree(ItemIconWidget* Widget)
{
    if (this->DetailLocations == nullptr) return;
    this->DetailLocations->clear();

    if (Widget == nullptr) return;

    const bool revealUncollected = AppConfig::GetRevealUncollectedItems();

    // Multiworld (option b): show placements DESTINED to the active world, wherever they are
    // physically placed. We therefore scan every world, filter by TargetWorld == active world
    // and, when the placement lives in another world, prefix the scene name with "World N".
    const uint8_t activeWorld = (uint8_t)(GetActiveWorld() + 1);
    const size_t numWorlds = GetNumWorlds();

    // Group entries by (PlacementWorld, Game, RenderScene). The render scene is what the
    // navigation command uses to load the right minimap, so it is the natural grouping key.
    struct SceneBucket
    {
        size_t PlacementWorld;
        int Game;
        uint32_t SceneID;
        QString DisplayName;
        QList<QPair<ObjectInfo*, bool /*collected*/>> Entries;
    };
    QHash<QString, SceneBucket> buckets;

    auto bucketKey = [](size_t World, int Game, uint32_t SceneID) {
        return QString::number(World) + ":" + QString::number(Game) + ":" + QString::number(SceneID);
    };

    auto addEntry = [&](size_t World, int Game, ObjectInfo* Object, bool Collected) {
        if (Object == nullptr) return;

        QString key = bucketKey(World, Game, Object->RenderScene);
        if (!buckets.contains(key))
        {
            SceneBucket b;
            b.PlacementWorld = World;
            b.Game = Game;
            b.SceneID = Object->RenderScene;
            const char* name = GetSceneName(Game, Object->RenderScene);
            QString sceneName = name ? QString::fromUtf8(name) : QString::number(Object->RenderScene);
            // Only tag the placement world when it differs from the world we are viewing,
            // so single-world / same-world placements read exactly as before.
            b.DisplayName = (numWorlds > 1 && (World + 1) != activeWorld)
                ? QString("World %1 — %2").arg(World + 1).arg(sceneName)
                : sceneName;
            buckets.insert(key, b);
        }

        SceneBucket& bucket = buckets[key];
        for (const auto& existing : bucket.Entries)
        {
            if (existing.first == Object) return;
        }
        bucket.Entries.append(qMakePair(Object, Collected));
    };

    // Always scan both games regardless of which page hosts the widget. An OoT-page
    // widget can legitimately list MM locations (e.g. shared items like Lens of Truth,
    // Bombchu) and vice-versa — the user wants every known placement of the item.
    static const struct { int Game; size_t Count; } Pages[] = {
        { OOT_GAME, OOT_NUM_SCENES },
        { MM_GAME,  MM_NUM_SCENES  },
    };

    // The same logical object may appear in two scene arrays:
    //  - "shadow" entries (Type=none) sitting in the home scene next to a Type-bearing
    //    canonical in the render scene (e.g. MM_POTION_SHOP "Item 2");
    //  - paired entries with the same Type in both arrays (e.g. cross-scene NPCs);
    //  - entries that exist only in the home scene with Type set and a different
    //    RenderScene (e.g. OoT Bazaar shop items living in OOT_BAZAAR but rendering
    //    in OOT_KAKARIKO_BAZAAR — there is no counterpart in the render scene).
    // We iterate every entry, drop the Type=none shadows, then dedup by
    // (World, Game, ObjectID, RenderScene) so paired entries collapse to a single leaf
    // while standalone cross-scene entries are still picked up.
    QSet<QString> seen;
    auto dedupKey = [](size_t World, int Game, const ObjectInfo* o) {
        return QString::number(World) + ":" + QString::number(Game) + ":" + QString::number(o->ObjectID) + ":" + QString::number(o->RenderScene) + ":" + QString::number((int)o->Type);
    };

    for (size_t world = 0; world < numWorlds; ++world)
    {
        for (const auto& page : Pages)
        {
            SceneObjects* scenes = GetWorldSceneObjects(world, page.Game);
            if (scenes == nullptr) continue;

            for (size_t s = 0; s < page.Count; ++s)
            {
                SceneObjects& scene = scenes[s];
                for (size_t o = 0; o < scene.NumOfObjs; ++o)
                {
                    ObjectInfo& obj = scene.Objects[o];

                    if (obj.TargetWorld != activeWorld) continue;   // not destined to the active world
                    if (obj.Type == ObjectType::none) continue;     // skip shadow placeholders
                    if (obj.Item == nullptr) continue;
                    if (!ItemMatchesWidget(Widget, page.Game, obj.Item)) continue;

                    bool collected = (obj.Status != ObjectState::Hidden);
                    if (!collected && !revealUncollected) continue;

                    QString key = dedupKey(world, page.Game, &obj);
                    if (seen.contains(key)) continue;
                    seen.insert(key);

                    addEntry(world, page.Game, &obj, collected);
                }
            }
        }
    }

    // "Starting Item" badge is the first row when the player begins the run
    // owning this item. It is purely informative — non-clickable, no scene to
    // navigate to — so it sits above any actual world location.
    if (Widget->IsStartingItem)
    {
        QTreeWidgetItem* startingNode = new QTreeWidgetItem(this->DetailLocations);
        startingNode->setText(0, "Starting Item");
        startingNode->setForeground(0, QBrush(QColor(248, 200, 120)));
        QColor startingBg(248, 200, 120);
        startingBg.setAlpha(48);
        startingNode->setBackground(0, QBrush(startingBg));
        QFont sf = startingNode->font(0);
        sf.setBold(true);
        sf.setItalic(true);
        startingNode->setFont(0, sf);
        startingNode->setFlags(startingNode->flags() & ~Qt::ItemIsSelectable);
    }

    if (buckets.isEmpty())
    {
        if (!Widget->IsStartingItem)
        {   // Without any starting-item row either, fall back to the empty-state placeholder.
            QTreeWidgetItem* placeholder = new QTreeWidgetItem(this->DetailLocations);
            placeholder->setText(0, revealUncollected ? "No known location" : "Not found yet");
            placeholder->setForeground(0, QBrush(QColor(122, 154, 191)));
            placeholder->setFlags(placeholder->flags() & ~Qt::ItemIsSelectable);
            if (this->DetailExpandToggle != nullptr) this->DetailExpandToggle->hide();
            return;
        }

        // We added a starting-item row but have no further locations: hide the
        // expand/collapse toggle since there is nothing to expand or collapse.
        if (this->DetailExpandToggle != nullptr) this->DetailExpandToggle->hide();
        return;
    }

    // Top-level grouping by game (OoT first, MM second), each game holds its
    // alphabetically-sorted scene buckets. The previous flat layout mixed both
    // games together which made it hard to scan when an item exists in both.
    static const struct { int Game; const char* Title; const char* AccentHex; } GameSections[] = {
        { OOT_GAME, "Ocarina of Time", "#4a9edb" },
        { MM_GAME,  "Majora's Mask",   "#9b5de5" },
    };

    for (const auto& gs : GameSections)
    {
        QList<QString> gameKeys;
        for (auto it = buckets.constBegin(); it != buckets.constEnd(); ++it)
        {
            if (it.value().Game == gs.Game) gameKeys.append(it.key());
        }
        if (gameKeys.isEmpty()) continue;

        std::sort(gameKeys.begin(), gameKeys.end(), [&](const QString& a, const QString& b) {
            return buckets.value(a).DisplayName.compare(buckets.value(b).DisplayName, Qt::CaseInsensitive) < 0;
        });

        // Game header row — bigger, bolder, fully tinted background to clearly
        // separate the two games.
        QTreeWidgetItem* gameNode = new QTreeWidgetItem(this->DetailLocations);
        QColor gameAccent(gs.AccentHex);
        QColor gameBg = gameAccent; gameBg.setAlpha(120);
        gameNode->setText(0, QString::fromUtf8(gs.Title));
        gameNode->setBackground(0, QBrush(gameBg));
        gameNode->setForeground(0, QBrush(QColor("#ffffff")));
        QFont gameFont = gameNode->font(0);
        gameFont.setBold(true);
        gameFont.setPointSize(gameFont.pointSize() + 1);
        gameNode->setFont(0, gameFont);
        gameNode->setFlags(gameNode->flags() & ~Qt::ItemIsSelectable);
        gameNode->setExpanded(true);

        for (const QString& key : gameKeys)
        {
            SceneBucket bucket = buckets.value(key);

            // Sort object leaves alphabetically inside the scene. Uncollected entries
            // (the bright "to-find" rows) come before collected ones so the actionable
            // list is at the top and crossed-off entries trail behind.
            std::sort(bucket.Entries.begin(), bucket.Entries.end(),
                [](const QPair<ObjectInfo*, bool>& a, const QPair<ObjectInfo*, bool>& b) {
                    if (a.second != b.second) return !a.second && b.second;
                    QString an = a.first && a.first->Name ? QString::fromUtf8(a.first->Name) : QString();
                    QString bn = b.first && b.first->Name ? QString::fromUtf8(b.first->Name) : QString();
                    return an.compare(bn, Qt::CaseInsensitive) < 0;
                });

            QTreeWidgetItem* sceneNode = new QTreeWidgetItem(gameNode);
            QColor accent(gs.AccentHex);
            QColor headerBg = accent; headerBg.setAlpha(48);

            QString suffix = QString(" (%1)").arg(bucket.Entries.size());
            sceneNode->setText(0, bucket.DisplayName + suffix);
            sceneNode->setBackground(0, QBrush(headerBg));
            sceneNode->setForeground(0, QBrush(accent));
            QFont catFont = sceneNode->font(0);
            catFont.setBold(true);
            sceneNode->setFont(0, catFont);
            sceneNode->setFlags(sceneNode->flags() & ~Qt::ItemIsSelectable);
            sceneNode->setExpanded(true);

            for (const auto& entry : bucket.Entries)
            {
                ObjectInfo* obj = entry.first;
                const bool collected = entry.second;

                QTreeWidgetItem* leaf = new QTreeWidgetItem(sceneNode);
                QString objectName = obj->Name ? QString::fromUtf8(obj->Name) : QString("Object");
                leaf->setData(0, kLocationGameRole, bucket.Game);
                leaf->setData(0, kLocationObjectRole, QVariant::fromValue<void*>(obj));
                leaf->setData(0, kLocationCollectedRole, collected);

                leaf->setText(0, objectName);

                if (collected)
                {   // Already-found entries read like crossed-off TODOs: italic,
                    // strike-through, faded icon. The color still encodes whether
                    // the entry was auto-collected or user-forced (amber).
                    leaf->setIcon(0, MakeFadedIcon(ResolveObjectIcon(obj)));

                    QFont f = leaf->font(0);
                    f.setItalic(true);
                    f.setStrikeOut(true);

                    if (obj->Status == ObjectState::Forced)
                    {
                        leaf->setForeground(0, QBrush(QColor(248, 200, 120, 170)));
                    }
                    else
                    {
                        leaf->setForeground(0, QBrush(QColor(180, 200, 225, 150)));
                    }

                    leaf->setFont(0, f);
                }
                else
                {   // Outstanding entries are the "to-find" list — keep them
                    // bright, fully opaque and unmodified so they grab attention.
                    leaf->setIcon(0, ResolveObjectIcon(obj));
                    leaf->setForeground(0, QBrush(QColor(230, 240, 255)));
                }

                QString tooltip = collected ? QString("%1 — collected").arg(objectName)
                                            : QString("%1 — not collected yet").arg(objectName);
                leaf->setToolTip(0, tooltip);
            }
        }
    }

    // Reveal the toggle now that the tree has content; reset state to expanded.
    if (this->DetailExpandToggle != nullptr)
    {
        this->DetailExpandToggle->show();
        this->DetailExpandToggle->setText("Collapse All");
        this->DetailLocationsExpanded = true;
    }
}


void ProgressionTab::OnLocationClicked(QTreeWidgetItem* Item, int /*Column*/)
{
    if (Item == nullptr) return;
    // Game and scene rows are never selectable, so the selectable flag alone
    // is enough to filter the click down to actual object leaves.
    if ((Item->flags() & Qt::ItemIsSelectable) == 0) return;

    int game = Item->data(0, kLocationGameRole).toInt();
    void* raw = Item->data(0, kLocationObjectRole).value<void*>();
    ObjectInfo* obj = static_cast<ObjectInfo*>(raw);
    if (obj == nullptr) return;

    if (this->WinOwner != nullptr)
    {
        this->WinOwner->NavigateToObject(game, obj);
    }
}


void ProgressionTab::RefreshCurrentDetail()
{
    if (this->CurrentDetailWidget != nullptr)
    {
        this->ShowDetailFor(this->CurrentDetailWidget);
        //this->BuildLocationTree(this->CurrentDetailWidget);
    }
}


QList<ItemIconWidget*> ProgressionTab::FindAllMatchingWidgets(const ItemInfo* Item) const
{
    QList<ItemIconWidget*> matches;
    if (Item == nullptr) return matches;

    const GameProgData* registries[] = {
        &this->OoTData, &this->MMData, &this->SoulsData, &this->CollectiblesData
    };

    for (const GameProgData* reg : registries)
    {   // The flat `All` list mirrors BuildPage's iteration of the static
        // ProgSection arrays, which is the declaration order. Progressive
        // items rely on that order to walk through their stages.
        for (ItemIconWidget* w : reg->All)
        {
            if (w == nullptr) continue;
            if (w->LookupKeys.contains(Item->ItemID))
            {
                matches.append(w);
            }
        }
    }

    return matches;
}


void ProgressionTab::OnItemFound(int Game, ObjectInfo* Object, const ItemInfo* Item, bool IsAddOp)
{
    if (Item == nullptr || Item->ItemName == nullptr) return;

    QList<ItemIconWidget*> matches = this->FindAllMatchingWidgets(Item);
    if (matches.isEmpty()) return;

    // Shared items propagate the update to every widget that lists them — both
    // the OoT and the MM mirror, plus any shared-only widget. The ItemInfo
    // gates eligibility (CanBeShared) and the active settings turn the share
    // on at runtime.
    const bool shared = Item->CanBeShared
        && this->RomSettings != nullptr
        && this->RomSettings->SharedItemIDs.contains(Item->ItemID);

    // Progressive items (sword / shield / clock stages, ...) are explicitly
    // listed by the spoiler-derived settings. When the flag is on, walk the
    // matching widgets in declaration order to advance / rewind one stage at a
    // time. Otherwise the item behaves like a normal collectible regardless of
    // how many widgets reference it.
    const bool progressive = this->RomSettings != nullptr
        && this->RomSettings->ProgressiveItemIDs.contains(Item->ItemID);

    // Advance to the next not-yet-found stage on Add, rewind to the most
    // recently found one on Remove. Operates on whatever subset of matches the
    // caller passes in — used both on the flat list (progressive-only) and on
    // each per-page subset (shared progressive items, e.g. shields).
    auto walkStages = [&](const QList<ItemIconWidget*>& Stages)
    {
        if (IsAddOp)
        {
            for (ItemIconWidget* w : Stages)
            {
                if (!w->Found) { w->MarkFound(Game, Object); return; }
            }
        }
        else
        {
            for (auto it = Stages.rbegin(); it != Stages.rend(); ++it)
            {
                if ((*it)->Found) { (*it)->MarkNotFound(Game, Object); return; }
            }
        }
    };

    if (shared && progressive)
    {   // Shared progressive items (e.g. shields) live on both the OoT and the
        // MM page — advancing the first not-yet-found stage on the flat list
        // would only progress one side. Walk each registry independently so
        // every mirror moves to the right stage in lockstep.
        const GameProgData* registries[] = {
            &this->OoTData, &this->MMData, &this->SoulsData, &this->CollectiblesData
        };
        for (const GameProgData* reg : registries)
        {
            QList<ItemIconWidget*> regMatches;
            for (ItemIconWidget* w : reg->All)
            {
                if (w != nullptr && w->LookupKeys.contains(Item->ItemID))
                {
                    regMatches.append(w);
                }
            }
            if (!regMatches.isEmpty()) walkStages(regMatches);
        }
        return;
    }

    if (shared || !progressive)
    {   // Either propagate to every mirror (shared) or update every match the
        // same way (non-progressive items only ever expect one widget per game,
        // and counter widgets keep accumulating on every hit).
        for (ItemIconWidget* w : matches)
        {
            if (IsAddOp) w->MarkFound(Game, Object);
            else         w->MarkNotFound(Game, Object);
        }
        return;
    }

    // Progressive only: every match lives on the same page so the flat walk is
    // enough to advance / rewind the correct stage.
    walkStages(matches);
}


void ProgressionTab::OnObjectForceStateChanged(int Game, ObjectInfo* Object)
{
    switch (Object->Status)
    {
        case ObjectState::Forced:
        {   // Add the object to the progression

            this->OnItemFound(Game, Object, Object->Item, true);
            break;
        }

        case ObjectState::Hidden:
        {   // Remove the object from the progression

            this->OnItemFound(Game, Object, Object->Item, false);
            break;
        }

        default:
        {   // If the state is collected it should be passed through this function

            return;
        }
    }

    this->RefreshCurrentDetail();
}


void ProgressionTab::ApplySettings(const Settings* NewRomSettings)
{
    this->RomSettings = NewRomSettings;

    GameProgData* registries[] = {
        &this->OoTData, &this->MMData, &this->SoulsData, &this->CollectiblesData
    };

    // When the 'songs' setting is 'all' (song notes shuffled individually), each song widget
    // accumulates a counter — one tick per note picked up. Any other value means the song is
    // delivered in one shot, so the counter badge would be meaningless and is hidden.
    bool songsAsCounter = false;
    if (this->RomSettings != nullptr)
    {
        auto songsIt = this->RomSettings->FilterSettings.find("songs");
        if (songsIt != this->RomSettings->FilterSettings.end())
        {
            songsAsCounter = songsIt.value().Value == ShuffleSetting::all;
        }
    }

    auto isSongIcon = [](EGameIcon Icon) -> bool
    {
        return Icon == EGameIcon::song
            || Icon == EGameIcon::song_green
            || Icon == EGameIcon::song_red
            || Icon == EGameIcon::song_blue
            || Icon == EGameIcon::song_purple
            || Icon == EGameIcon::song_orange
            || Icon == EGameIcon::song_yellow;
    };

    for (GameProgData* reg : registries)
    {
        for (ItemIconWidget* w : reg->All)
        {
            if (w == nullptr) continue;

            // Re-show by default so a settings change that re-enables a
            // category brings its widgets back into the layout.
            w->setVisible(true);

            // Refresh IsCounter from the active 'songs' setting before checking visibility:
            // settings can flip between runs without rebuilding the page.
            if (isSongIcon(w->Icon))
            {
                w->IsCounter = songsAsCounter;
            }

            if (this->RomSettings == nullptr) continue;

            for (uint32_t id : w->LookupKeys)
            {
                if (this->RomSettings->DisabledItemIDs.contains(id))
                {
                    w->setVisible(false);
                    break;
                }
            }
        }
    }

    // Reflow each section's grid so disabled widgets do not leave a gap. A
    // simple setVisible(false) keeps the cell reserved (the column has a
    // min-width matching the icon size); detaching from the layout and
    // re-adding only the still-visible widgets gives a tight pack.
    this->RepackVisibleWidgets();

    // Hide whole sections that lost every widget and whole pages whose
    // sections all collapsed, so the dashboard never shows an empty header
    // or a blank tab.
    this->UpdateSectionAndTabVisibility();
}


void ProgressionTab::RepackVisibleWidgets()
{
    const int columns = 16;

    for (GridSection& section : this->Sections)
    {
        if (section.Grid == nullptr) continue;

        // Detach every widget from the grid first; addWidget below will
        // re-anchor only those that should still be displayed. removeWidget
        // does not touch the parent pointer, so the widget keeps living
        // under gridHost — Qt will paint it again as soon as it is back in
        // the layout.
        for (ItemIconWidget* w : section.Widgets)
        {
            if (w == nullptr) continue;
            section.Grid->removeWidget(w);
        }

        int row = 0;
        int col = 0;
        for (ItemIconWidget* w : section.Widgets)
        {
            if (w == nullptr) continue;

            // isHidden() reports the explicit setVisible(false) state set by
            // ApplySettings (it is independent of the parent QStackedWidget's
            // current page, so widgets on a non-active page are still
            // considered "to be packed" here — only DisabledItemIDs hides
            // them at this layer).
            if (w->isHidden()) continue;

            section.Grid->addWidget(w, row, col);

            ++col;
            if (col >= columns)
            {
                col = 0;
                ++row;
            }
        }
    }
}


void ProgressionTab::UpdateSectionAndTabVisibility()
{
    if (this->SubTabBar == nullptr || this->PageStack == nullptr) return;

    const int pageCount = this->PageStack->count();

    // Step 1: per-section header/grid host visibility + accumulate per-page
    // "has any visible section" flags in a fixed-size array indexed by the
    // page index stored on each GridSection.
    QVector<bool> pageHasContent(pageCount, false);

    for (const GridSection& section : this->Sections)
    {
        if (section.Header == nullptr || section.GridHost == nullptr) continue;

        bool hasVisible = false;
        for (ItemIconWidget* w : section.Widgets)
        {
            if (w != nullptr && !w->isHidden())
            {
                hasVisible = true;
                break;
            }
        }

        section.Header->setVisible(hasVisible);
        section.GridHost->setVisible(hasVisible);

        if (hasVisible
            && section.PageIndex >= 0
            && section.PageIndex < pageCount)
        {
            pageHasContent[section.PageIndex] = true;
        }
    }

    // Step 2: hide tabs whose page has no remaining content. If the active
    // tab is the one being hidden, fall back to the first still-visible tab
    // so the user never lands on a blank page.
    const int currentIdx = this->SubTabBar->currentIndex();
    int firstVisible = -1;

    for (int i = 0; i < this->SubTabBar->count(); ++i)
    {
        const bool visible = (i < pageCount) ? pageHasContent[i] : true;
        this->SubTabBar->setTabVisible(i, visible);
        if (visible && firstVisible < 0) firstVisible = i;
    }

    if (currentIdx >= 0
        && currentIdx < pageCount
        && !pageHasContent[currentIdx]
        && firstVisible >= 0)
    {
        this->SubTabBar->setCurrentIndex(firstVisible);
    }
}


void ProgressionTab::RebuildFromSceneObjects()
{
    // Replay every collected/forced object from the central SceneObjects arrays.
    // Counters are summed naturally because OnItemFound increments on each call.
    this->ResetProgress();

    // Starting items are applied first so the world replay below can detect
    // them as already-found and progressive items advance to the right stage.
    if (this->RomSettings != nullptr && !this->RomSettings->StartingItemIDs.isEmpty())
    {
        GameProgData* registries[] = {
            &this->OoTData, &this->MMData, &this->SoulsData, &this->CollectiblesData
        };

        for (GameProgData* reg : registries)
        {
            for (ItemIconWidget* w : reg->All)
            {
                if (w == nullptr) continue;
                for (uint32_t id : w->LookupKeys)
                {
                    if (this->RomSettings->StartingItemIDs.contains(id))
                    {
                        w->IsStartingItem = true;
                        for (uint32_t i = 0; i < this->RomSettings->StartingItemIDs.value(id); i++)
                        {   // Mark the item found as many time necessary

                            w->MarkFound(0, nullptr);
                        }
                        break;
                    }
                }
            }
        }
    }

    static const struct { int Game; size_t Count; } Pages[] = {
        { OOT_GAME, OOT_NUM_SCENES },
        { MM_GAME,  MM_NUM_SCENES  },
    };

    // Multiworld (option b): the active world's progression is every placement DESTINED to
    // that world, wherever it is physically placed. So we scan all worlds and keep only the
    // placements whose TargetWorld matches the active world (1-based). In single / coop there
    // is one world and every TargetWorld defaults to 1, so this matches everything as before.
    const uint8_t activeWorld = (uint8_t)(GetActiveWorld() + 1);
    const size_t numWorlds = GetNumWorlds();

    // A single logical object can appear in both its home scene's array and the
    // render scene's array (cross-scene NPCs, paired shop slots, ...). Without
    // deduping, OnItemFound would be called twice for the same pickup and the
    // counter widget (e.g. Gold Skulltula Tokens) would over-report. The world is part
    // of the key because the same scene/objectID coordinate exists in every world clone
    // as a genuinely distinct placement. Same key as BuildLocationTree so the count and
    // the location tree stay perfectly in sync.
    QSet<QString> seen;

    for (size_t world = 0; world < numWorlds; ++world)
    {
        for (const auto& page : Pages)
        {
            SceneObjects* scenes = GetWorldSceneObjects(world, page.Game);
            if (scenes == nullptr) continue;

            for (size_t s = 0; s < page.Count; ++s)
            {
                SceneObjects& scene = scenes[s];
                for (size_t o = 0; o < scene.NumOfObjs; ++o)
                {
                    ObjectInfo& obj = scene.Objects[o];
                    if (obj.TargetWorld != activeWorld) continue;   // not destined to the active world
                    if (obj.Status == ObjectState::Hidden) continue;
                    if (obj.Item == nullptr) continue;
                    if (obj.Type == ObjectType::none) continue;     // shadow placeholder

                    QString key = QString::number(world) + ":" +
                                  QString::number(page.Game) + ":" +
                                  QString::number(obj.ObjectID) + ":" +
                                  QString::number(obj.RenderScene) + ":" +
                                  QString::number((int)obj.Type);
                    if (seen.contains(key)) continue;
                    seen.insert(key);

                    this->OnItemFound(page.Game, &obj, obj.Item, true);
                }
            }
        }
    }
}


void ProgressionTab::ResetProgress()
{
    GameProgData* registries[] = {
        &this->OoTData, &this->MMData, &this->SoulsData, &this->CollectiblesData
    };

    for (GameProgData* reg : registries)
    {
        for (ItemIconWidget* w : reg->All)
        {
            if (w == nullptr) continue;
            w->IsStartingItem = false;
            w->ResetFound();
        }
    }
}

#pragma endregion
