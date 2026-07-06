#include "Common.h"
#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "UI/AppConfig.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/SceneEntrance.h"
#include "UI/SettingsTab.h"
#include "UI/ProgressionTab.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QEnterEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QStatusBar>
#include <QTabBar>
#include <QTextStream>
#include <QDebug>

namespace {

/*
*   Lightweight custom-painted progress bar used inside the tab content.
*   QProgressBar embedded via QTabBar::setTabButton can fall back to system colors
*   on some Windows themes; this widget paints itself directly so the accent color
*   is guaranteed regardless of stylesheet propagation.
*/
class TabProgressLine : public QWidget
{
public:
    QColor TrackColor = QColor("#060c16");  // Background color of the unfilled track.
    QColor ChunkColor;                      // Filled portion color (game accent).
    int Value = 0;                          // Current value in [0, 100].

    TabProgressLine(const QColor& Chunk, QWidget* Parent = nullptr) : QWidget(Parent), ChunkColor(Chunk)
    {
        this->setFixedHeight(3);
        this->setMinimumWidth(70);
    }

    /* Set the progress value (0-100 percent) and trigger a repaint. */
    void SetValue(int Pct)
    {
        Value = qBound(0, Pct, 100);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);

        QRectF bg(0, 0, width(), height());
        p.setBrush(TrackColor);
        p.drawRoundedRect(bg, 1, 1);

        if (Value > 0)
        {
            QRectF chunk(0, 0, (qreal)width() * Value / 100.0, height());
            p.setBrush(ChunkColor);
            p.drawRoundedRect(chunk, 1, 1);
        }
    }
};


/*
*   Two-segment progress line for the global counter: OoT progress in blue,
*   MM progress in violet, sharing the same total. Segments are painted side
*   by side and the rounded outline is achieved via a clipping path so the
*   junction between the two colors stays sharp.
*/
class DualProgressLine : public QWidget
{
public:
    QColor TrackColor = QColor("#060c16");
    QColor OoTColor = QColor("#4a9edb");
    QColor MMColor = QColor("#9b5de5");
    int OoTFound = 0;
    int MMFound = 0;
    int Total = 0;

    DualProgressLine(QWidget* Parent = nullptr) : QWidget(Parent)
    {
        this->setFixedHeight(4);
        this->setMinimumWidth(140);
    }

    /* Update the per-game collected counts and overall total, then repaint. */
    void SetValues(int OoTCollected, int MMCollected, int TotalObjects)
    {
        OoTFound = qMax(0, OoTCollected);
        MMFound = qMax(0, MMCollected);
        Total = qMax(0, TotalObjects);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);

        QRectF bg(0, 0, width(), height());

        // Clip to a rounded rect so the two colored segments share a clean rounded outline.
        QPainterPath clip;
        clip.addRoundedRect(bg, 2, 2);
        p.setClipPath(clip);

        p.fillRect(bg, TrackColor);

        if (Total > 0)
        {
            qreal totalW = (qreal)width();
            qreal ootW = totalW * OoTFound / Total;
            qreal mmW = totalW * MMFound / Total;

            if (ootW > 0)
            {
                p.fillRect(QRectF(0, 0, ootW, height()), OoTColor);
            }
            if (mmW > 0)
            {
                p.fillRect(QRectF(ootW, 0, mmW, height()), MMColor);
            }
        }
    }
};

} // namespace


#pragma region StatusPill

StatusPill::StatusPill(const QString& ActiveText, const QString& InactiveText, QWidget* Parent)
    : QWidget(Parent), ActiveText(ActiveText), InactiveText(InactiveText)
{
    this->setCursor(Qt::PointingHandCursor);
    this->setAttribute(Qt::WA_Hover, true);
    this->setMouseTracking(true);
}


void StatusPill::SetActive(bool NewActive)
{
    if (this->Active == NewActive) return;
    this->Active = NewActive;
    this->updateGeometry();
    this->update();
}


QSize StatusPill::sizeHint() const
{
    // Match the font configured in paintEvent so the metrics actually account for
    // the Medium weight (otherwise the regular-weight metrics under-estimate the
    // width and the last glyph gets clipped).
    QFont f = this->font();
    f.setPointSizeF(f.pointSizeF() - 0.5);
    f.setWeight(QFont::Medium);
    QFontMetrics fm(f);

    // Layout (mirrors paintEvent):
    //   [2px adjust] [8px left pad] [8px dot] [6px gap] [text] [8px right pad] [2px adjust] + safety.
    // Reserve room for the longer of the two label states so the box never has to
    // shrink + clip when toggling between active/inactive.
    const int textWidth = qMax(fm.horizontalAdvance(this->ActiveText),
                               fm.horizontalAdvance(this->InactiveText));
    const int w = 2 + 8 + 8 + 6 + textWidth + 8 + 2 + 4 /* safety */;
    return QSize(w, 22);
}


void StatusPill::paintEvent(QPaintEvent* /*Event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);

    // Background pill: subtly shaded card so the indicator reads as a clickable
    // chip without competing with the main UI.
    QColor pillBg = this->Hovered ? QColor(255, 255, 255, 28) : QColor(255, 255, 255, 14);
    QRectF pillRect = this->rect().adjusted(2, 2, -2, -2);
    p.setBrush(pillBg);
    p.drawRoundedRect(pillRect, 10, 10);

    // Status dot.
    const int dotSize = 8;
    int dotX = static_cast<int>(pillRect.left()) + 8;
    int dotY = static_cast<int>(pillRect.center().y()) - (dotSize / 2);
    QColor dotColor = this->Active ? QColor(101, 224, 154) : QColor(255, 105, 105);
    p.setBrush(dotColor);
    p.drawEllipse(QRect(dotX, dotY, dotSize, dotSize));

    // Soft glow around the dot when active.
    if (this->Active)
    {
        QColor glow = dotColor; glow.setAlpha(70);
        p.setBrush(glow);
        p.drawEllipse(QRect(dotX - 2, dotY - 2, dotSize + 4, dotSize + 4));
        p.setBrush(dotColor);
        p.drawEllipse(QRect(dotX, dotY, dotSize, dotSize));
    }

    // Label.
    QFont f = this->font();
    f.setPointSizeF(f.pointSizeF() - 0.5);
    f.setWeight(QFont::Medium);
    p.setFont(f);

    QColor textColor = this->Active ? QColor(221, 238, 255) : QColor(170, 190, 215);
    p.setPen(textColor);

    QRect textRect(dotX + dotSize + 6, this->rect().top(),
                   this->rect().right() - (dotX + dotSize + 6) - 8, this->rect().height());
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, this->Active ? this->ActiveText : this->InactiveText);
}


void StatusPill::mousePressEvent(QMouseEvent* Event)
{
    if (Event->button() == Qt::LeftButton)
    {
        emit this->Clicked();
    }
    QWidget::mousePressEvent(Event);
}


void StatusPill::enterEvent(QEnterEvent* Event)
{
    this->Hovered = true;
    this->update();
    QWidget::enterEvent(Event);
}


void StatusPill::leaveEvent(QEvent* Event)
{
    this->Hovered = false;
    this->update();
    QWidget::leaveEvent(Event);
}

#pragma endregion


OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    this->ui.setupUi(this);

    // The main tab widget
    this->TabWidget = new QTabWidget;

    // The log / lauch tab
    this->Log = new LogTab(this);

    // Create OoT and MM game tabs
    this->OoTTab = new GameTab(OOT_GAME, this);
    this->OoTTab->Owner = this;
    this->MMTab = new GameTab(MM_GAME, this);
    this->MMTab->Owner = this;
    this->EntTab = new EntranceTab(3, this->TabWidget);
    //this->EntranceTab->Owner = this;

    // Items collection dashboard
    this->ProgTab = new ProgressionTab(this, this);

    // Add the tabs to the widget : Log -> OoT -> MM -> Progression -> Entrances
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(this->OoTTab, this->OoTTab->TabName);
    this->TabWidget->addTab(this->MMTab, this->MMTab->TabName);
    this->TabWidget->addTab(this->EntTab, this->EntTab->TabName);
    this->TabWidget->addTab(this->ProgTab, "Progression");

    // Custom tab content:
    //   [Game name label] | [Counter label]
    //                     | [Progress bar ]
    auto makeTabWidget = [](const QString& gameName, const QString& accentColor,
                            QLabel** counterOut, QWidget** progressOut) {
        QWidget* w = new QWidget();
        w->setAttribute(Qt::WA_TranslucentBackground);
        w->setAttribute(Qt::WA_TransparentForMouseEvents);

        QHBoxLayout* mainLayout = new QHBoxLayout(w);
        mainLayout->setContentsMargins(4, 2, 4, 2);
        mainLayout->setSpacing(8);

        QLabel* gameLabel = new QLabel(gameName);
        gameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        gameLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px; font-weight: 600;").arg(accentColor));
        gameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(2);

        QLabel* counter = new QLabel("0 / 0");
        counter->setAttribute(Qt::WA_TransparentForMouseEvents);
        counter->setStyleSheet("background: transparent; color: #7a9abf; font-size: 10px;");
        counter->setAlignment(Qt::AlignRight | Qt::AlignBottom);

        TabProgressLine* pb = new TabProgressLine(QColor(accentColor));
        pb->setAttribute(Qt::WA_TransparentForMouseEvents);

        rightLayout->addWidget(counter);
        rightLayout->addWidget(pb);

        mainLayout->addWidget(gameLabel);
        mainLayout->addLayout(rightLayout);

        *counterOut = counter;
        *progressOut = pb;
        return w;
    };
    QWidget* oottWidget = makeTabWidget("OoT", "#4a9edb", &this->OoTTabLabel, &this->OoTTabProgress);
    QWidget* mmtWidget = makeTabWidget("MM",  "#9b5de5", &this->MMTabLabel,  &this->MMTabProgress);
    this->TabWidget->tabBar()->setTabButton(this->TabWidget->indexOf(this->OoTTab), QTabBar::LeftSide, oottWidget);
    this->TabWidget->tabBar()->setTabButton(this->TabWidget->indexOf(this->MMTab), QTabBar::LeftSide, mmtWidget);
    // The custom widget carries everything, hide the native tab text to avoid duplication
    this->TabWidget->setTabText(this->TabWidget->indexOf(this->OoTTab), "");
    this->TabWidget->setTabText(this->TabWidget->indexOf(this->MMTab), "");

    // Global "Total X/Y" indicator on the right side of the tab bar:
    //   [Total]            ────────────────
    //   [X / Y]            blue (OoT) | violet (MM)
    {
        QWidget* globalWidget = new QWidget();
        globalWidget->setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout* globalLayout = new QHBoxLayout(globalWidget);
        globalLayout->setContentsMargins(8, 2, 12, 2);
        globalLayout->setSpacing(10);

        // Multiworld: a world selector that switches every view (maps, entrances, progression)
        // to the chosen world. Hidden until a multiworld spoiler allocates more than one world.
        this->WorldSelector = new QComboBox();
        this->WorldSelector->setToolTip("Select which world to display");
        this->WorldSelector->hide();
        connect(this->WorldSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &OoTMMComboTracker::OnWorldSelected);
        globalLayout->addWidget(this->WorldSelector, 0, Qt::AlignVCenter);

        QVBoxLayout* counterCol = new QVBoxLayout();
        counterCol->setContentsMargins(0, 0, 0, 0);
        counterCol->setSpacing(0);

        QLabel* totalLabel = new QLabel("Total");
        totalLabel->setStyleSheet("background: transparent; color: #7a9abf; font-size: 9px; font-weight: 600; letter-spacing: 0.08em;");
        totalLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);

        this->GlobalCounter = new QLabel("<span style='color:#ddeeff; font-size:14px; font-weight:700;'>0</span><span style='color:#7a9abf; font-size:11px;'>/0</span>");
        this->GlobalCounter->setTextFormat(Qt::RichText);
        this->GlobalCounter->setStyleSheet("background: transparent;");
        this->GlobalCounter->setAlignment(Qt::AlignRight | Qt::AlignTop);

        counterCol->addWidget(totalLabel);
        counterCol->addWidget(this->GlobalCounter);

        DualProgressLine* dualBar = new DualProgressLine();
        this->GlobalProgress = dualBar;

        globalLayout->addLayout(counterCol);
        globalLayout->addWidget(dualBar, 0, Qt::AlignVCenter);

        this->TabWidget->setCornerWidget(globalWidget, Qt::TopRightCorner);
    }

    // Bottom status bar: 3 clickable status pills (tracking / auto-save / reveal)
    // on the left, last-activity label on the right.
    {
        QStatusBar* sb = this->statusBar();
        sb->setSizeGripEnabled(false);

        this->TrackingPill = new StatusPill("Tracking active", "Tracking inactive", sb);
        this->AutoSavePill = new StatusPill("Auto-save on", "Auto-save off", sb);
        this->RevealPill   = new StatusPill("Items revealed", "Items hidden", sb);

        this->TrackingPill->SetActive(false); // updated via UpdateTrackingState when tracking starts/stops
        this->AutoSavePill->SetActive(AppConfig::GetAutoSave());
        this->RevealPill->SetActive(AppConfig::GetRevealUncollectedItems());

        this->TrackingPill->setToolTip("Click to start or stop the auto-tracker");
        this->AutoSavePill->setToolTip("Click to toggle automatic save on each update");
        this->RevealPill->setToolTip("Click to show / hide item names for uncollected objects");

        sb->addWidget(this->TrackingPill);
        sb->addWidget(this->AutoSavePill);
        sb->addWidget(this->RevealPill);

        this->LastActivityLabel = new QLabel(sb);
        this->LastActivityLabel->setStyleSheet("background: transparent; color: #7a9abf; font-size: 11px;");
        this->LastActivityLabel->setText("");
        sb->addPermanentWidget(this->LastActivityLabel, 1);

        // Forward pill clicks to the existing handlers so menu actions and pills stay in sync.
        // For toggle pills we just flip the menu action's checked state — the toggled signal
        // on the action already wires AppConfig + pill->SetActive on its own.
        connect(this->TrackingPill, &StatusPill::Clicked, this->Log, &LogTab::PressLaunchButton);
        connect(this->AutoSavePill, &StatusPill::Clicked, this, [this]()
        {
            this->ui.actionAutoSaving->setChecked(!AppConfig::GetAutoSave());
        });
        connect(this->RevealPill, &StatusPill::Clicked, this, [this]()
        {
            this->ui.actionRevealUncollectedItems->setChecked(!AppConfig::GetRevealUncollectedItems());
        });
    }

    // Update game tabs name
    this->UpdateTabNameText(0);
    this->UpdateTabNameText(1);
    this->EntTab->RefreshName();
    this->setCentralWidget(this->TabWidget);
    this->setWindowTitle("OoTMMCombo Auto Tracker");
    this->setWindowIcon(QIcon("./Resources/Logo.ico"));

    // Loads default value
    this->ui.actionAutoSnapView->setChecked(AppConfig::GetAutoSnapView());
    this->ui.actionAutoZoom->setChecked(AppConfig::GetAutoZoom());
    this->ui.actionAutoSaving->setChecked(AppConfig::GetAutoSave());
    this->ui.actionAutoLoadTrackingFile->setChecked(AppConfig::GetAutoLoadTrackingFile());
    this->ui.actionAutoLoadSpoilerLog->setChecked(AppConfig::GetAutoLoadSpoilerLog());
    this->ui.actionHideCollectedFromObjectList->setChecked(AppConfig::GetHideCollectedFromObjectList());
    this->ui.actionHideCollectedFromMap->setChecked(AppConfig::GetHideCollectedFromMap());
    this->ui.actionRevealUncollectedItems->setChecked(AppConfig::GetRevealUncollectedItems());

    // Recent files sub menu
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        QAction* recentAction = new QAction(this);
        recentAction->setVisible(false);
        connect(recentAction, &QAction::triggered, this, [this, i](){
            this->LoadGameScenes(this->RecentFiles[i]);
        });
        this->RecentActions.append(recentAction);
        this->ui.menuRecents->addAction(recentAction);
    }

    this->RecentFiles = AppConfig::GetRecentFiles();
    this->UpdateRecentFiles();

    // Order matters: the spoiler must be loaded FIRST (it resets the scene objects and rebuilds
    // the per-world placements), then the tracking save is applied ON TOP to restore the actual
    // collected state. Doing it the other way round would let LoadGameSpoiler's ResetSceneObjects
    // wipe everything the save just restored.
    if (AppConfig::GetAutoLoadSpoilerLog())
    {
        this->LoadGameSpoiler(AppConfig::GetLastSpoilerLogPath());
    }

    if (AppConfig::GetAutoLoadTrackingFile() && this->RecentFiles.size())
    {
        this->LoadGameScenes(this->RecentFiles.front());
    }

    // Connections
    connect(this->TabWidget, &QTabWidget::currentChanged, this, [this](int index)
    {
        /*if (index == 1) this->ApplyGameTheme(OOT_GAME);
        if (index == 2) this->ApplyGameTheme(MM_GAME);*/
    });
    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyEntranceFound, this, &OoTMMComboTracker::UpdateTrackedEntrance);
    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
    connect(this->ui.actionSaveSession, &QAction::triggered, this->Log, &LogTab::SaveTracking);
    connect(this->ui.actionLoadSession, &QAction::triggered, this->Log, &LogTab::LoadTracking);
    connect(this->ui.actionLoadSpoilerLog, &QAction::triggered, this->Log, &LogTab::LoadSpoiler);
    connect(this->ui.actionResetTracking, &QAction::triggered, this->Log, &LogTab::ResetTracking);
    connect(this->ui.actionStartTracking, &QAction::triggered, this->Log, &LogTab::PressLaunchButton);
    connect(this->ui.actionAutoSnapView, &QAction::toggled, this, &AppConfig::SetAutoSnapView);
    connect(this->ui.actionAutoZoom, &QAction::toggled, this, &AppConfig::SetAutoZoom);
    connect(this->ui.actionHideCollectedFromMap, &QAction::toggled, this, &OoTMMComboTracker::UpdateObjectMapVisibility);
    connect(this->ui.actionHideCollectedFromObjectList, &QAction::toggled, this, &OoTMMComboTracker::UpdateObjectListVisibility);
    connect(this->ui.actionRevealUncollectedItems, &QAction::toggled, this, &OoTMMComboTracker::UpdateRevealUncollectedItems);
    connect(this->ui.actionAutoSaving, &QAction::toggled, this, [this](bool checked)
    {
        AppConfig::SetAutoSave(checked);
        if (this->AutoSavePill) this->AutoSavePill->SetActive(checked);
    });
    connect(this->ui.actionAbout, &QAction::triggered, this, &OoTMMComboTracker::ShowAboutDialog);
    connect(this->ui.actionAutoLoadTrackingFile, &QAction::triggered, this, &AppConfig::SetAutoLoadTrackingFile);
    connect(this->ui.actionAutoLoadSpoilerLog, &QAction::triggered, this, &AppConfig::SetAutoLoadSpoilerLog);

    // Add "Edit Settings..." entry to the ROM Settings menu so clicking the menu
    // entry opens the modal SettingsTab editor.
    QAction* editSettingsAction = this->ui.menuROM_Settings->addAction("Edit Settings...");
    connect(editSettingsAction, &QAction::triggered, this, [this]()
    {
        SettingsTab dialog(this, this);
        dialog.exec();
    });
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    delete this->Log;
    delete this->OoTTab;
    delete this->MMTab;
    delete this->ProgTab;
    delete this->TabWidget;
}

void OoTMMComboTracker::ShowAboutDialog()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("OoTMMCombo Auto Tracker<br>Version 2.0.5<br>&copy; 2025-2026 Loupimo<br><br>git repository: <a href='https://github.com/Loupimo/OoTMMCombo-Tracker'>https://github.com/Loupimo/OoTMMCombo-Tracker</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<br><br>Thanks for testing to :<br><br>- Wild<br><br>-DataSkywalker");
    msgBox.exec();
}

void OoTMMComboTracker::ApplyGameTheme(int GameID)
{
    QFile file("./Resources/Styles/DualRealm.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString qss = QTextStream(&file).readAll();
    file.close();

    if (GameID == MM_GAME) {
        qss.replace("#4a9edb", "#9b5de5");
        qss.replace("rgba(74, 158, 219", "rgba(155, 93, 229");
        qss.replace("#1a4a7a", "#3a1560");
        qss.replace("#0d2a4a", "#1e0a38");
        qss.replace("#080f1a", "#0d0812");
        qss.replace("#0d1827", "#130d1f");
        qss.replace("#1a3050", "#2a1545");
    }

    qApp->setStyleSheet(qss);
}

void OoTMMComboTracker::CreatePath(QString PathToCreate)
{
    QDir dir(PathToCreate);

    if (!dir.exists())
    {
        if (dir.mkpath("."))
        {
            MultiLogger::LogMessage("Created path : %s", PathToCreate.toStdString().c_str());
        }
        else
        {
            MultiLogger::LogMessage("Can't created path : %s", PathToCreate.toStdString().c_str());
        }
    }
}

#pragma region Object related

void OoTMMComboTracker::UpdateTabNameText(int TabID)
{
    GameTab* activeTab = nullptr;
    QWidget* activeProgress = nullptr;
    QLabel* activeLabel = nullptr;

    if (TabID == OOT_GAME)
    {   // OoT

        activeTab = this->OoTTab;
        activeProgress = this->OoTTabProgress;
        activeLabel = this->OoTTabLabel;
    }
    else
    {   // MM

        activeTab = this->MMTab;
        activeProgress = this->MMTabProgress;
        activeLabel = this->MMTabLabel;
    }

    if (activeLabel != nullptr)
    {
        activeLabel->setText(QString("%1 / %2").arg(activeTab->FoundObjects).arg(activeTab->TotalObjects));
    }
    else
    {
        QString finalName = BuildCountLabel(activeTab->TabName, activeTab->FoundObjects, activeTab->TotalObjects);
        this->TabWidget->setTabText(TabID + 1, finalName);
    }

    if (activeProgress != nullptr)
    {
        int pct = activeTab->TotalObjects > 0 ? (100 * activeTab->FoundObjects) / activeTab->TotalObjects : 0;
        static_cast<TabProgressLine*>(activeProgress)->SetValue(pct);
    }

    if (this->GlobalCounter != nullptr)
    {   // Skip the inactive game when the user picked an OoT-only or MM-only ROM,
        // otherwise the leftover counters from the other tab leak into the Total.

        bool includeOoT = this->ROMSettings.Game != ROMGame::mm;
        bool includeMM = this->ROMSettings.Game != ROMGame::oot;

        int ootFound = includeOoT ? this->OoTTab->FoundObjects : 0;
        int mmFound = includeMM ? this->MMTab->FoundObjects : 0;
        int ootTotal = includeOoT ? this->OoTTab->TotalObjects : 0;
        int mmTotal = includeMM ? this->MMTab->TotalObjects : 0;

        int totalFound = ootFound + mmFound;
        int totalObjs = ootTotal + mmTotal;
        this->GlobalCounter->setText(QString(
            "<span style='color:#ddeeff; font-size:14px; font-weight:700;'>%1</span>"
            "<span style='color:#7a9abf; font-size:11px;'>/%2</span>")
            .arg(totalFound).arg(totalObjs));

        if (this->GlobalProgress != nullptr)
        {
            static_cast<DualProgressLine*>(this->GlobalProgress)->SetValues(
                ootFound, mmFound, totalObjs);
        }
    }

    // Keep the per-scene header in sync with the counters
    if (activeTab != nullptr && activeTab->GameMaps != nullptr)
    {
        activeTab->GameMaps->RefreshSceneHeader();
    }
}


void OoTMMComboTracker::UpdateObjectMapVisibility(bool NewValue)
{
    AppConfig::SetHideCollectedFromMap(NewValue);
    this->UpdateObjectVisibilityForAllGames();
}


void OoTMMComboTracker::UpdateObjectListVisibility(bool NewValue)
{
    AppConfig::SetHideCollectedFromObjectList(NewValue);
    this->UpdateObjectVisibilityForAllGames();
}


void OoTMMComboTracker::UpdateRevealUncollectedItems(bool NewValue)
{
    AppConfig::SetRevealUncollectedItems(NewValue);

    // Just trigger a repaint — the data on each ObjectItemTree is already up-to-date,
    // it's only the delegate that branches on the option to display "???" or the item.
    if (this->OoTTab && this->OoTTab->GameMaps && this->OoTTab->GameMaps->ObjectList)
    {
        this->OoTTab->GameMaps->ObjectList->viewport()->update();
    }
    if (this->MMTab && this->MMTab->GameMaps && this->MMTab->GameMaps->ObjectList)
    {
        this->MMTab->GameMaps->ObjectList->viewport()->update();
    }

    if (this->RevealPill) this->RevealPill->SetActive(NewValue);

    if (this->ProgTab != nullptr)
    {   // Reveal toggle changes which uncollected locations are shown in the detail tree.
        this->ProgTab->RefreshCurrentDetail();
    }
}


void OoTMMComboTracker::UpdateObjectVisibilityForAllGames()
{
    this->OoTTab->UpdateObjectVisibility();
    this->MMTab->UpdateObjectVisibility();
}

void OoTMMComboTracker::NavigateToObject(int Game, ObjectInfo* Object)
{
    if (Object == nullptr) return;

    GameTab* target = nullptr;
    if (Game == OOT_GAME)      target = this->OoTTab;
    else if (Game == MM_GAME)  target = this->MMTab;

    if (target == nullptr || target->GameMaps == nullptr) return;

    // Bring the relevant game tab to the foreground so the user actually sees the
    // map switch when the navigation is triggered from the Progression dashboard.
    int idx = this->TabWidget->indexOf(target);
    if (idx >= 0)
    {
        this->TabWidget->setCurrentIndex(idx);
    }

    target->GameMaps->FocusObject(Object);
}


void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound, ItemSource Source, int FromWorld, int ToWorld)
{
    // Route by event source using the world ids the game itself provides. No manual "my world"
    // is needed: the game already decides what to apply, so we just place each event in the
    // right world.
    //
    //   map      <- the world where the item is physically collected (the "from" world)
    //   progress <- the destination world the item is granted to (the "to" world)
    //
    //   HookNothing : local "nothing" drop (never sent over the wire) -> map of the local world
    //   HookItem    : real local item -> only authoritative in single mode (else network owns it)
    //   NetOut      : a check the local player collected for someone   -> map of "from"
    //   NetIn       : a ledger entry applied (from -> to)              -> map of "from", progress of "to"
    //
    // Coop is special: the whole team shares one inventory, so a local collection (NetOut) AND a
    // teammate's collection echoed back (NetIn) both have to update the map AND the progression.
    // Because OoTMM fires both ITEM OUT and ITEM IN for an already-discovered shared check, the
    // progression is credited only on the first Hidden -> Collected transition (idempotency guard
    // below) so a shared check is counted exactly once.
    //
    // The placement physically lives in the "from" world and carries TargetWorld == "to". The
    // destination world's progression is rebuilt from placements with TargetWorld == that world
    // and Status != Hidden, so marking the "from" placement collected feeds both the map of
    // "from" and the progression of "to" — no separate bookkeeping per world.
    const bool singleWorld = (GetNumWorlds() <= 1);

    // Auto-detect the local world from the network stream so hook-only events (nothing drops,
    // which never travel over the wire) land on the right map without any manual setting.
    if (Source == ItemSource::NetOut && FromWorld > 0)      this->ROMSettings.LocalWorld = (size_t)FromWorld;
    else if (Source == ItemSource::NetIn && ToWorld > 0)    this->ROMSettings.LocalWorld = (size_t)ToWorld;

    const int localWorld = (int)this->ROMSettings.LocalWorld;

    bool updateMap = false;
    bool updateProgress = false;
    int mapWorld = -1;        // 1-based world whose map gets the collected mark
    int progressWorld = -1;   // 1-based world whose progression gets credited

    switch (Source)
    {
        case ItemSource::HookNothing:
            updateMap = true; mapWorld = localWorld;
            if (this->ROMSettings.Mode == GameMode::single)
            {
                updateProgress = true; progressWorld = localWorld;
            }
            break;

        case ItemSource::HookItem:
            // In single mode the hook is the only source; otherwise the network ledger owns it.
            if (this->ROMSettings.Mode == GameMode::single)
            {
                updateMap = true; mapWorld = localWorld;
                updateProgress = true; progressWorld = localWorld;
            }
            break;

        case ItemSource::NetOut:
            updateMap = true; mapWorld = (FromWorld > 0 ? FromWorld : localWorld);
            // Coop shares one inventory across the team, so a locally collected check must also
            // credit the progression (the idempotency guard prevents double counting with NetIn).
            if (this->ROMSettings.Mode == GameMode::coop)
            {
                updateProgress = true; progressWorld = (FromWorld > 0 ? FromWorld : localWorld);
            }
            break;

        case ItemSource::NetIn:
            updateMap = true; mapWorld = (FromWorld > 0 ? FromWorld : localWorld);
            updateProgress = true; progressWorld = (ToWorld > 0 ? ToWorld : localWorld);
            break;
    }

    // A single world (single / coop seeds) collapses everything onto world 0.
    if (singleWorld) { mapWorld = 1; progressWorld = 1; }

    if (!updateMap && !updateProgress)
    {
        return;
    }

    // Resolve the placement inside the "from" world (where it physically lives).
    const size_t mapIdx = mapWorld > 0 ? (size_t)(mapWorld - 1) : 0;
    ObjectInfo* worldObj = FindObjectInWorld(mapIdx, Game, ObjectFound);

    // Idempotency guard: a shared coop check is reported by both ITEM OUT and ITEM IN, and the
    // same placement can be re-applied. Credit the progression only on the first transition out
    // of Hidden so it is counted exactly once.
    const bool wasHidden = (worldObj == nullptr) || (worldObj->Status == ObjectState::Hidden);

    if (updateMap && mapIdx == GetActiveWorld())
    {
        // The placement world is on screen: let ItemFound perform the Hidden -> Collected
        // transition AND refresh the live counters (scene / region / global), exactly like
        // before. Pre-marking the status here would make ItemFound skip the count (it only
        // counts objects still flagged Hidden).
        switch (Game)
        {
            case OOT_GAME: this->OoTTab->ItemFound(worldObj, ItemFound); break;
            case MM_GAME:  this->MMTab->ItemFound(worldObj, ItemFound); break;
            default: break;
        }
    }
    else if (worldObj != nullptr)
    {
        // The placement lives in a world that is not displayed (or this is a progress-only
        // event): just record the collected state so the map / dashboard pick it up when that
        // world is selected and rebuilt from the scene arrays.
        worldObj->Status = ObjectState::Collected;
    }

    // Live progression increment only when the item is destined to the world on screen AND this
    // is the first time the placement is collected (prevents the coop ITEM OUT + ITEM IN double
    // count). Other worlds are re-derived from Status on the next world switch.
    const size_t progressIdx = progressWorld > 0 ? (size_t)(progressWorld - 1) : 0;
    if (updateProgress && wasHidden && this->ProgTab != nullptr && progressIdx == GetActiveWorld())
    {
        this->ProgTab->OnItemFound(Game, ObjectFound, ItemFound, true);
        this->ProgTab->RefreshCurrentDetail();
    }

    if (AppConfig::GetAutoSave())
    {
        this->CreatePath(AppConfig::GetAutoSavePath());
        GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
    }

    if (this->LastActivityLabel && ObjectFound)
    {
        const QString itemName = (ItemFound && ItemFound->ItemName) ? QString(ItemFound->ItemName) : QString("Item");
        const QString locName = (Game == OOT_GAME ? QString("OoT, ") : QString("MM, ")) + GetSceneName(Game, ObjectFound->Scene) + QString(": ") + QString(ObjectFound->Name);
        this->LastActivityLabel->setText(QString("Last item: %1 @ %2").arg(itemName, locName));
    }
}

#pragma endregion


#pragma region Entrance related

void OoTMMComboTracker::UpdateTrackedEntrance(SceneEntranceUpdate OutEntrance, SceneEntranceUpdate InEntrance)
{
    this->EntTab->UpdateEntranceWay(OutEntrance.Game, OutEntrance.SceneID, OutEntrance.EntranceID, OutEntrance.Link);
    this->EntTab->UpdateEntranceWay(InEntrance.Game, InEntrance.SceneID, InEntrance.EntranceID, InEntrance.Link);
    if (AppConfig::GetAutoSave())
    {
        this->CreatePath(AppConfig::GetAutoSavePath());
        GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
    }

    if (this->LastActivityLabel)
    {
        SceneMetaInfo* fromMeta = GetSceneMetaInfo(OutEntrance.SceneID, OutEntrance.Game);
        SceneMetaInfo* toMeta   = GetSceneMetaInfo(InEntrance.SceneID, InEntrance.Game);

        const QString fromName = (fromMeta && fromMeta->Name) ? QString(fromMeta->Name) : QString::number(OutEntrance.SceneID);
        const QString toName   = (toMeta   && toMeta->Name)   ? QString(toMeta->Name)   : QString::number(InEntrance.SceneID);

        this->LastActivityLabel->setText(QString("Last entrance: %1 → %2").arg(fromName, toName));
    }
}

#pragma endregion

#pragma region Saving / Loading / Menu

void OoTMMComboTracker::UpdateRecentFiles()
{
    if (this->RecentFiles.size() > 0)
    {
        this->ui.menuRecents->setEnabled(true);
    }
    else
    {
        this->ui.menuRecents->setEnabled(true);
        return;
    }
    
    int numRecentFiles = std::min((byte)this->RecentFiles.size(), this->MaxRecentFiles);

    for (byte i = 0; i < this->MaxRecentFiles; ++i)
    {
        if (i < numRecentFiles)
        {
            QString text = this->RecentFiles[i]; //QFileInfo(this->RecentFiles[i]).fileName();
            this->RecentActions[i]->setText(text);
            this->RecentActions[i]->setVisible(true);
        }
        else
        {
            this->RecentActions[i]->setVisible(false);
        }
    }
}


void OoTMMComboTracker::AddRecentFile(const QString& filePath)
{
    this->RecentFiles.removeAll(filePath);
    this->RecentFiles.prepend(filePath);

    while (this->RecentFiles.size() > this->MaxRecentFiles)
    {
        this->RecentFiles.removeLast();
    }

    this->UpdateRecentFiles();
    AppConfig::SetRecentFiles(this->RecentFiles);
}


void OoTMMComboTracker::UpdateTrackingState(QString NewState, QIcon NewIcon)
{
    this->ui.actionStartTracking->setText(NewState);
    this->ui.actionStartTracking->setIcon(NewIcon);

    if (this->TrackingPill)
    {
        // The action toggles between "Start Tracking" (=> currently inactive) and
        // "Stop Tracking" (=> currently active).
        this->TrackingPill->SetActive(NewState.compare("Stop Tracking", Qt::CaseInsensitive) == 0);
    }
}


void OoTMMComboTracker::ApplySettings()
{
    // The settings filters mutate ObjectInfo entries (exclusions, vanilla items, ...) on the
    // ACTIVE world's arrays. In multiworld every world must receive the same treatment, so we
    // walk all worlds, applying the filter to each, then restore the active world. The active
    // world is applied last so the FilterManager state matches what is currently displayed.
    const size_t savedActive = GetActiveWorld();
    const size_t numWorlds = GetNumWorlds();

    auto applyForActiveWorld = [this]()
    {
        switch (this->ROMSettings.Game)
        {
            case ROMGame::oot:
                this->ROMSettings.ApplyOoTSettingsToFilter(this->OoTTab->GameMaps->FilterButton);
                break;
            case ROMGame::mm:
                this->ROMSettings.ApplyMMSettingsToFilter(this->MMTab->GameMaps->FilterButton);
                break;
            case ROMGame::ootmm:
            default:
                this->ROMSettings.ApplySettings(this->OoTTab->GameMaps->FilterButton, this->MMTab->GameMaps->FilterButton);
                break;
        }
    };

    for (size_t w = 0; w < numWorlds; w++)
    {
        if (w == savedActive) continue;   // applied last, below
        SetActiveWorld(w);
        applyForActiveWorld();
    }
    SetActiveWorld(savedActive);
    applyForActiveWorld();

    // Tab visibility only depends on the ROM game combination, not the world.
    switch (this->ROMSettings.Game)
    {
        case ROMGame::oot:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), true);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), false);
            break;
        }

        case ROMGame::mm:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), false);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), true);
            break;
        }

        case ROMGame::ootmm:
        default:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), true);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), true);
            break;
        }
    }

    // Propagate visibility / share / starting-item toggles to the progression
    // dashboard. The actual starting-item pre-marking happens later inside
    // RebuildFromSceneObjects (called via RefreshTracker) so it survives the
    // reset-and-replay cycle.
    if (this->ProgTab != nullptr)
    {
        this->ProgTab->ApplySettings(&this->ROMSettings);
    }
}


void OoTMMComboTracker::RefreshTracker()
{
    this->RefreshWorldSelector();

    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
    this->EntTab->RefreshEntranceTab();

    if (this->ProgTab != nullptr)
    {   // Replay every non-hidden object so the dashboard reflects the current scene state
        // (covers save load and full-reset paths in one place).

        this->ProgTab->RebuildFromSceneObjects();
    }
}


void OoTMMComboTracker::RefreshWorldSelector()
{
    if (this->WorldSelector == nullptr) return;

    const size_t numWorlds = GetNumWorlds();

    // Single world: nothing to choose, hide the selector entirely.
    if (numWorlds <= 1)
    {
        this->WorldSelector->hide();
        return;
    }

    // Rebuild the entries only when the world count changed, to avoid re-entrancy with
    // the currentIndexChanged signal while we are repopulating.
    const QSignalBlocker blocker(this->WorldSelector);
    if ((size_t)this->WorldSelector->count() != numWorlds)
    {
        this->WorldSelector->clear();
        for (size_t w = 0; w < numWorlds; w++)
        {
            this->WorldSelector->addItem(QString("World %1").arg(w + 1), (int)w);
        }
    }
    this->WorldSelector->setCurrentIndex((int)GetActiveWorld());
    this->WorldSelector->show();
}


void OoTMMComboTracker::OnWorldSelected(int Index)
{
    if (Index < 0) return;

    SetActiveWorld((size_t)Index);

    // Re-point every view at the newly active world. RefreshWorldSelector inside
    // RefreshTracker keeps the combo's selected index in sync without re-emitting.
    this->RefreshTracker();
}


void OoTMMComboTracker::LoadGameScenes(QString FilePath)
{
    QFile loadFile(FilePath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        MultiLogger::LogMessage("Can't open file: %s\n", FilePath.toStdString().c_str());
        return;
    }

    this->AddRecentFile(FilePath);

    // Load file data
    QByteArray data = loadFile.readAll();

    // Check tracker version
    uint32_t version = 0;
    memcpy_s(&version, sizeof(version), data.data(), sizeof(version));
    size_t offset = sizeof(version);

    switch ((TrackerVersion)version)
    {
        case TrackerVersion::V2_0:
        {   // Current format: re-allocate the worlds, load each one's scene objects, then entrances.

            offset = this->ROMSettings.LoadFileSettings(&data, offset);
            offset = LoadAllWorlds(&data, offset);
            offset = LoadEntrances(&data, offset, TrackerVersion::V2_0);
            break;
        }

        case TrackerVersion::V1_0:
        default:
        {
            LoadSceneObjects(&data, 0);
            break;
        }
    }

    loadFile.close();

    MultiLogger::LogMessage("File loaded: %s\n", FilePath.toStdString().c_str());

    this->RefreshTracker();
}


void OoTMMComboTracker::LoadGameSpoiler(QString FilePath)
{
    QFile file(FilePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cout << "Can't open file:" << file.errorString().toStdString();
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll(); // Read all file
    file.close();

    // Split with "===" to find the right section
    QStringList sections = content.split("===========================================================================", Qt::SkipEmptyParts, Qt::CaseSensitive);

    // Check that we have the correct number of sections
    if (sections.size() < 3)
    {
        std::cout << "The file does not have the correct number of sections.";
        return;
    }

    // Loads Settings section
    this->ROMSettings = Settings();
    this->ROMSettings.ParseSettings(sections[0]);

    // Detect the ROM build from the spoiler's "Version:" line. This is the most reliable source
    // (the alternative is the DLL-reported game version, see MemoryReader). The build decides
    // whether raw in-game item IDs need translating to the tracker's internal numbering: dev
    // builds match it as-is, stable builds report lower IDs that ResolveRawItemID shifts up.
    {
        QRegularExpression versionReg("^Version: (.+)", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator versionIt = versionReg.globalMatch(content);
        if (versionIt.hasNext())
        {
            QString version = versionIt.next().captured(1).trimmed();
            this->ROMSettings.Version = version.startsWith("dev") ? ROMVersion::dev : ROMVersion::stable;
        }
    }

    // Publish it to the global the item-ID translation reads (works across the UI and the
    // multiplayer threads without threading the version through every call site).
    SetActiveROMVersion(this->ROMSettings.Version, true);

    // The local world is auto-detected later from the network stream (see UpdateTrackedObject);
    // it defaults to world 1 until the first network event arrives.
    this->ROMSettings.LocalWorld = 1;

    // Auto-enable multiplayer for coop / multiworld seeds, disable it for single seeds.
    // The host / port fields and the persisted config follow the checkbox automatically.
    if (this->Log != nullptr)
    {
        this->Log->SetMultiplayerEnabled(this->ROMSettings.Mode != GameMode::single);
    }

    // Multiworld spoilers add a per-world level: each world is a "  World N (hash)" header
    // and its locations are indented one extra level. We parse EVERY world (not just the
    // local one) so the world selector can show any world's map and progression. Each
    // world's block is dedented by two spaces so it matches the single-world layout that
    // ParseWorldLocations understands; the per-item "Player N " prefix is parsed there.
    QString locationSection = sections[2];
    bool isMultiworld = QRegularExpression("^  World \\d", QRegularExpression::MultilineOption)
                            .match(locationSection).hasMatch();

    // Start from a clean slate: clear any item / status left in the template arrays by a
    // previous session, since InitWorlds clones the templates into every world.
    ResetSceneObjects();

    if (!isMultiworld)
    {   // Single / coop: one world, backed by the static template arrays.

        InitWorlds(1);
        SetActiveWorld(0);
        this->ParseWorldLocations(locationSection, 0, false);
    }
    else
    {   // Multiworld: split the section into per-world blocks ("World N" header), then
        // fill world N-1 from each block.

        QRegularExpression worldHeader("^  World (\\d+)", QRegularExpression::MultilineOption);

        // First pass: collect each world's dedented block, keyed by its 1-based number.
        QMap<int, QStringList> worldBlocks;
        int currentWorld = -1;
        for (const QString& line : locationSection.split('\n'))
        {
            QRegularExpressionMatch hm = worldHeader.match(line);
            if (hm.hasMatch())
            {
                currentWorld = hm.captured(1).toInt();
                worldBlocks[currentWorld];   // Ensure the key exists even for an empty world.
                continue;                    // The header line itself is not a location.
            }
            if (currentWorld > 0)
            {
                worldBlocks[currentWorld].append(line.startsWith("  ") ? line.mid(2) : line);
            }
        }

        // Allocate one world per parsed block (worlds are 1-based in the spoiler, so the
        // count is the highest world number seen). InitWorlds clones the templates.
        int maxWorld = worldBlocks.isEmpty() ? 1 : worldBlocks.lastKey();
        InitWorlds((size_t)maxWorld);

        for (auto it = worldBlocks.constBegin(); it != worldBlocks.constEnd(); ++it)
        {
            this->ParseWorldLocations(it.value().join('\n'), (size_t)(it.key() - 1), true);
        }

        // Show the first world by default; the user switches worlds with the selector.
        SetActiveWorld(0);
    }

    this->ApplySettings();
    this->RefreshTracker();

    AppConfig::SetLastSpoilerLogPath(FilePath);
}


void OoTMMComboTracker::ParseWorldLocations(const QString& LocationBlock, size_t WorldIndex, bool IsMultiworld)
{
    // Regex to split strings by location
    QRegularExpression reg("^\\s{2}(.+:(?:\n\\s{4}.*)+)\n*", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LocationBlock);

    QStringList maps;
    while (it.hasNext())
    {   // Fill the maps array with all the gathered matches

        QRegularExpressionMatch match = it.next();
        maps.append(match.captured(1));
    }

    const QHash<QString, QPair<uint32_t, uint32_t>> spoilerMap = this->Log->GetSpoilerMap();

    for (QString map : maps)
    {   // Browse all maps

        // All objects start with four spaces
        QStringList objects = map.split("    ");

        // Regex to split map / object and their associated item
        reg = QRegularExpression("^([\\w'-]+(?:\\s[\\w'-]+)*)", QRegularExpression::MultilineOption);

        // Get the map location
        it = reg.globalMatch(objects[0]);
        QString mapName = it.next().captured(1);

        uint32_t sceneID = spoilerMap[mapName].first;                                   // Get the scene ID that match the spoiler log location
        uint32_t sceneGame = spoilerMap[mapName].second;                                // Get the game the header scene belongs to

        for (qsizetype i = 1; i < objects.size(); i++)
        {   // Browse all the spoiler scene objects

            QStringList spoilObject = objects[i].split(": ");                           // Left part is object name, right is the item it contains
            spoilObject[0] = spoilObject[0].replace("\n", "");                          // Sometimes the object has a line break, just get rid of it

            uint8_t targetWorld = (uint8_t)(WorldIndex + 1);                            // Default: item belongs to this world (1-based).
            if (IsMultiworld && spoilObject.size() > 1)
            {   // Multiworld items carry the destination player ("Player 2 Compass (Water Temple)").
                // Capture the destination world, then strip the prefix so the name matches the item table.

                QRegularExpressionMatch pm = QRegularExpression("^Player (\\d+) ").match(spoilObject[1]);
                if (pm.hasMatch())
                {
                    targetWorld = (uint8_t)pm.captured(1).toInt();
                    spoilObject[1].remove(QRegularExpression("^Player \\d+ "));
                }
            }

            if (spoilObject.size() < 2)
            {   // Malformed line without an item to assign, skip it

                continue;
            }

            QByteArray objNameBytes = spoilObject[0].toUtf8();                          // Keep the buffer alive while it is used as a C string below
            const char* objName = objNameBytes.constData();
            const ItemInfo* item = FindItemByName(spoilObject[1]);

            // Fast path: the object lives in the scene named by its header (always true without entrance shuffle).
            if (this->AssignSpoilerObjectInScene(WorldIndex, sceneGame, sceneID, objName, item, targetWorld))
            {
                continue;
            }

            // Fallback: entrance shuffle can list a location under a different scene header than the one
            // it natively belongs to (moved grottos, relocated boss lairs, ...). The Location string is
            // globally unique and carries its game prefix, so scan every scene of that game to find it.
            uint32_t objGame = spoilObject[0].startsWith("MM ") ? MM_GAME : OOT_GAME;
            if (!this->AssignSpoilerObjectAnyScene(WorldIndex, objGame, objName, item, targetWorld))
            {   // Still unresolved even after the cross-scene scan (previously this failed silently)

                qWarning() << "[Spoiler] Unresolved location under" << mapName << ":" << spoilObject[0];
            }
        }
    }
}

bool OoTMMComboTracker::AssignSpoilerObjectInScene(size_t WorldIndex, uint32_t Game, uint32_t SceneID, const char* ObjName, const ItemInfo* Item, uint8_t TargetWorld)
{
    SceneObjects* gameSceneObj = GetWorldSceneObjects(WorldIndex, Game);
    GameLayout sceneActiveLayout = GetSceneMetaInfo(SceneID, Game)->ActiveLayout;
    bool found = false;

    for (size_t j = 0; j < gameSceneObj[SceneID].NumOfObjs; j++)
    {   // Browse all scenes objects

        ObjectInfo* object = &gameSceneObj[SceneID].Objects[j];

        // Some layout have the same object name but different object position. We need to check if the active layout match the object one in order to fill the right one.
        if (object->Layout != GameLayout::all && object->Layout != sceneActiveLayout)
        {   // The layout does not match, skip this object

            continue;
        }

        if (strcmp(object->Location, ObjName) != 0)
        {   // Not the object we are looking for

            continue;
        }

        // We have found the object, modify its item
        object->Item = Item;
        object->TargetWorld = TargetWorld;
        found = true;

        if (object->RenderScene != SceneID)
        {   // The current object will never be rendered, we need to update its counter part

            for (size_t k = 0; k < gameSceneObj[object->RenderScene].NumOfObjs; k++)
            {   // Find the object in the rendered scene

                if (strcmp(gameSceneObj[object->RenderScene].Objects[k].Location, object->Location) == 0)
                {   // Object found

                    gameSceneObj[object->RenderScene].Objects[k].Item = Item;
                    gameSceneObj[object->RenderScene].Objects[k].TargetWorld = TargetWorld;
                    break;
                }
            }
        }

        if (object->Type == ObjectType::none)
        {   // The object is in the good renderer however in some cases it might be not rendered (e.g. MM Mountain village Spring / Winter)

            continue;   // We should do another loop and not break in order to find the real rendered item
        }

        break;
    }

    return found;
}

bool OoTMMComboTracker::AssignSpoilerObjectAnyScene(size_t WorldIndex, uint32_t Game, const char* ObjName, const ItemInfo* Item, uint8_t TargetWorld)
{
    size_t numScenes = (Game == MM_GAME) ? MM_NUM_SCENES : OOT_NUM_SCENES;

    for (size_t scene = 0; scene < numScenes; scene++)
    {   // Browse every scene of the game until we find the object's real home

        if (this->AssignSpoilerObjectInScene(WorldIndex, Game, (uint32_t)scene, ObjName, Item, TargetWorld))
        {
            return true;
        }
    }

    return false;
}

#pragma endregion
