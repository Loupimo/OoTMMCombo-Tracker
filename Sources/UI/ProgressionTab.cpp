#include "UI/ProgressionTab.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/AppConfig.h"
#include "UI/ProgressionEntry.h"
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

        const int columns = 16;
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
            //widget->LookupKey = entry.LookupKey ? QString::fromUtf8(entry.LookupKey) : QString();
            widget->LookupKeys = widget->LookupKeys.unite(entry.LookupKeys);
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
    QWidget* ootPage = this->BuildPage(OoTSections, NumOoTSections, this->OoTData, OOT_GAME);
    this->PageStack->addWidget(ootPage);

    QWidget* mmPage = this->BuildPage(MMSections, NumMMSections, this->MMData, MM_GAME);
    this->PageStack->addWidget(mmPage);

    // Souls span both games; the detail panel uses OOT_GAME by convention to resolve scene names.
    QWidget* soulsPage = this->BuildPage(SoulsSections, NumSoulsSections, this->SoulsData, OOT_GAME);
    this->PageStack->addWidget(soulsPage);
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
        "letter-spacing: 0.05em; border-bottom: 1px solid #1a3050; padding-bottom: 3px;");

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
    layout->addWidget(this->DetailLocationsHeader);
    layout->addWidget(this->DetailLocations, 1);

    return panel;
}


void ProgressionTab::ShowDetailFor(ItemIconWidget* Widget)
{
    if (Widget == nullptr) return;

    this->CurrentDetailWidget = Widget;

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

} // namespace


void ProgressionTab::BuildLocationTree(ItemIconWidget* Widget)
{
    if (this->DetailLocations == nullptr) return;
    this->DetailLocations->clear();

    if (Widget == nullptr) return;

    const bool revealUncollected = AppConfig::GetRevealUncollectedItems();

    // Group entries by (Game, RenderScene). The render scene is what the navigation
    // command uses to load the right minimap, so it is the natural grouping key.
    struct SceneBucket
    {
        int Game;
        uint32_t SceneID;
        QString DisplayName;
        QList<QPair<ObjectInfo*, bool /*collected*/>> Entries;
    };
    QHash<QString, SceneBucket> buckets;

    auto bucketKey = [](int Game, uint32_t SceneID) {
        return QString::number(Game) + ":" + QString::number(SceneID);
    };

    auto addEntry = [&](int Game, ObjectInfo* Object, bool Collected) {
        if (Object == nullptr) return;

        QString key = bucketKey(Game, Object->RenderScene);
        if (!buckets.contains(key))
        {
            SceneBucket b;
            b.Game = Game;
            b.SceneID = Object->RenderScene;
            const char* name = GetSceneName(Game, Object->RenderScene);
            b.DisplayName = name ? QString::fromUtf8(name) : QString::number(Object->RenderScene);
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
    // (Game, ObjectID, RenderScene) so paired entries collapse to a single leaf
    // while standalone cross-scene entries are still picked up.
    QSet<QString> seen;
    auto dedupKey = [](int Game, const ObjectInfo* o) {
        return QString::number(Game) + ":" + QString::number(o->ObjectID) + ":" + QString::number(o->RenderScene) + ":" + QString::number((int)o->Type);
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

                if (obj.Type == ObjectType::none) continue;     // skip shadow placeholders
                if (obj.Item == nullptr) continue;
                if (!ItemMatchesWidget(Widget, page.Game, obj.Item)) continue;

                bool collected = (obj.Status != ObjectState::Hidden);
                if (!collected && !revealUncollected) continue;

                QString key = dedupKey(page.Game, &obj);
                if (seen.contains(key)) continue;
                seen.insert(key);

                addEntry(page.Game, &obj, collected);
            }
        }
    }

    // Alphabetic ordering of scenes; ties (same name across games) keep OoT before MM.
    QList<QString> orderedKeys = buckets.keys();
    std::sort(orderedKeys.begin(), orderedKeys.end(), [&](const QString& a, const QString& b) {
        const SceneBucket& ba = buckets.value(a);
        const SceneBucket& bb = buckets.value(b);
        int cmp = ba.DisplayName.compare(bb.DisplayName, Qt::CaseInsensitive);
        if (cmp != 0) return cmp < 0;
        return ba.Game < bb.Game;
    });

    if (orderedKeys.isEmpty())
    {
        QTreeWidgetItem* placeholder = new QTreeWidgetItem(this->DetailLocations);
        placeholder->setText(0, revealUncollected ? "No known location" : "Not found yet");
        placeholder->setForeground(0, QBrush(QColor(122, 154, 191)));
        placeholder->setFlags(placeholder->flags() & ~Qt::ItemIsSelectable);
        return;
    }

    for (const QString& key : orderedKeys)
    {
        SceneBucket bucket = buckets.value(key);

        // Sort object leaves alphabetically inside the scene. Collected entries keep
        // priority over uncollected ones so the user sees what they have first.
        std::sort(bucket.Entries.begin(), bucket.Entries.end(),
            [](const QPair<ObjectInfo*, bool>& a, const QPair<ObjectInfo*, bool>& b) {
                if (a.second != b.second) return a.second && !b.second;
                QString an = a.first && a.first->Name ? QString::fromUtf8(a.first->Name) : QString();
                QString bn = b.first && b.first->Name ? QString::fromUtf8(b.first->Name) : QString();
                return an.compare(bn, Qt::CaseInsensitive) < 0;
            });

        QTreeWidgetItem* sceneNode = new QTreeWidgetItem(this->DetailLocations);
        QString accentHex = (bucket.Game == OOT_GAME) ? "#4a9edb" : "#9b5de5";
        QColor accent(accentHex);
        QColor headerBg = accent; headerBg.setAlpha(64);

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
            leaf->setText(0, objectName);
            leaf->setIcon(0, ResolveObjectIcon(obj));
            leaf->setData(0, kLocationGameRole, bucket.Game);
            leaf->setData(0, kLocationObjectRole, QVariant::fromValue<void*>(obj));
            leaf->setData(0, kLocationCollectedRole, collected);

            if (collected)
            {
                if (obj->Status == ObjectState::Forced)
                {   // User-forced entries echo MapTab's amber palette.
                    leaf->setForeground(0, QBrush(QColor(248, 200, 120)));
                }
                else
                {   // Auto-collected entries: dim, neutral.
                    leaf->setForeground(0, QBrush(QColor(204, 218, 240, 200)));
                }
            }
            else
            {   // Uncollected (only shown when RevealUncollectedItems is on).
                leaf->setForeground(0, QBrush(QColor(141, 162, 192, 200)));
                QFont f = leaf->font(0);
                f.setItalic(true);
                leaf->setFont(0, f);
            }

            QString tooltip = collected ? QString("%1 — collected").arg(objectName)
                                        : QString("%1 — not collected yet").arg(objectName);
            leaf->setToolTip(0, tooltip);
        }
    }
}


void ProgressionTab::OnLocationClicked(QTreeWidgetItem* Item, int /*Column*/)
{
    if (Item == nullptr || Item->parent() == nullptr) return; // Top-level scene rows are not navigable.
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
        this->BuildLocationTree(this->CurrentDetailWidget);
    }
}


void ProgressionTab::OnItemFound(int Game, ObjectInfo* Object, const ItemInfo* Item)
{
    if (Item == nullptr || Item->ItemName == nullptr) return;

    //QString normalized = NormalizeItemName(QString::fromUtf8(Item->ItemName));
    GameProgData* primary = nullptr;
    if (Game == OOT_GAME)      primary = &this->OoTData;
    else if (Game == MM_GAME)  primary = &this->MMData;

    // 1. Primary lookup: per-game icon hash, disambiguated by LookupKey when needed.
    ItemIconWidget* widget = nullptr;
    if (primary != nullptr)
    {
        //widget = FindByIcon(*primary, Item->RenderType, normalized);
        widget = FindByIcon(*primary, Item);
    }

    // 2. Souls span both games -> they live in their own registry.
    if (widget == nullptr)
    {
        //widget = FindByIcon(this->SoulsData, Item->RenderType, normalized);
        widget = FindByIcon(this->SoulsData, Item);
    }

    // 3. Fallback: Item->RenderType cannot be matched (e.g. RenderType=none for progressive items).
    //    Scan the per-game flat list and match by LookupKey only.
    if (widget == nullptr && primary != nullptr)
    {
        //widget = FindByLookupKey(*primary, normalized);
        widget = FindByLookupKey(*primary, Item);
    }

    if (widget != nullptr)
    {
        widget->MarkFound(Game, Object);
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

    // A single logical object can appear in both its home scene's array and the
    // render scene's array (cross-scene NPCs, paired shop slots, ...). Without
    // deduping, OnItemFound would be called twice for the same pickup and the
    // counter widget (e.g. Gold Skulltula Tokens) would over-report. Use the
    // same dedup key as BuildLocationTree so the count and the location tree
    // stay perfectly in sync.
    QSet<QString> seen;

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
                if (obj.Status == ObjectState::Hidden) continue;
                if (obj.Item == nullptr) continue;
                if (obj.Type == ObjectType::none) continue;     // shadow placeholder

                QString key = QString::number(page.Game) + ":" +
                              QString::number(obj.ObjectID) + ":" +
                              QString::number(obj.RenderScene) + ":" +
                              QString::number((int)obj.Type);
                if (seen.contains(key)) continue;
                seen.insert(key);

                this->OnItemFound(page.Game, &obj, obj.Item);
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
