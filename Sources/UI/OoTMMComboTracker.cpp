#include "Common.h"
#include "Combo/Objects.h"
#include "UI/AppConfig.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/SceneEntrance.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QTabBar>
#include <QTextStream>

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
    
    // Add the tabs to the widget : Log -> OoT -> MM
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(this->OoTTab, this->OoTTab->TabName);
    this->TabWidget->addTab(this->MMTab, this->MMTab->TabName);
    this->TabWidget->addTab(this->EntTab, this->EntTab->TabName);

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
    if (AppConfig::GetAutoLoadTrackingFile() && this->RecentFiles.size())
    {
        this->LoadGameScenes(this->RecentFiles.front());
    }

    if (AppConfig::GetAutoLoadSpoilerLog())
    {
        this->LoadGameSpoiler(AppConfig::GetLastSpoilerLogPath());
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
    connect(this->ui.actionAutoSaving, &QAction::toggled, this, &AppConfig::SetAutoSave);
    connect(this->ui.actionAbout, &QAction::triggered, this, &OoTMMComboTracker::ShowAboutDialog);
    connect(this->ui.actionAutoLoadTrackingFile, &QAction::triggered, this, &AppConfig::SetAutoLoadTrackingFile);
    connect(this->ui.actionAutoLoadSpoilerLog, &QAction::triggered, this, &AppConfig::SetAutoLoadSpoilerLog);
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    delete this->Log;
    delete this->OoTTab;
    delete this->MMTab;
    delete this->TabWidget;
}

void OoTMMComboTracker::ShowAboutDialog()
{
    QMessageBox::about(this, "About", "OoTMMCombo Auto Tracker\nVersion 2.0\n(c) 2025-2026 Loupimo");
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
    {
        int totalFound = this->OoTTab->FoundObjects + this->MMTab->FoundObjects;
        int totalObjs = this->OoTTab->TotalObjects + this->MMTab->TotalObjects;
        this->GlobalCounter->setText(QString(
            "<span style='color:#ddeeff; font-size:14px; font-weight:700;'>%1</span>"
            "<span style='color:#7a9abf; font-size:11px;'>/%2</span>")
            .arg(totalFound).arg(totalObjs));

        if (this->GlobalProgress != nullptr)
        {
            static_cast<DualProgressLine*>(this->GlobalProgress)->SetValues(
                this->OoTTab->FoundObjects, this->MMTab->FoundObjects, totalObjs);
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
}


void OoTMMComboTracker::UpdateObjectVisibilityForAllGames()
{
    this->OoTTab->UpdateObjectVisibility();
    this->MMTab->UpdateObjectVisibility();
}

void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound)
{
    switch (Game)
    {
        case OOT_GAME:
        {
            this->OoTTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
            }
            break;
        }
        case MM_GAME:
        {
            this->MMTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

#pragma endregion


#pragma region Entrance related

void OoTMMComboTracker::UpdateTrackedEntrance(SceneEntranceUpdate* OutEntrance, SceneEntranceUpdate* InEntrance)
{
    this->EntTab->UpdateEntranceWay(OutEntrance->Game, OutEntrance->SceneID, OutEntrance->EntranceID, OutEntrance->Link);
    this->EntTab->UpdateEntranceWay(InEntrance->Game, InEntrance->SceneID, InEntrance->EntranceID, InEntrance->Link);
    if (AppConfig::GetAutoSave())
    {
        this->CreatePath(AppConfig::GetAutoSavePath());
        GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
    }
    /*switch (Game)
    {
        case OOT_GAME:
        {
            this->OoTTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
            }
            break;
        }
        case MM_GAME:
        {
            this->MMTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->ROMSettings);
            }
            break;
        }
        default:
        {
            break;
        }
    }*/
}
/*
void OoTMMComboTracker::UpdateTabNameText(int TabID)
{
    GameTab* activeTab = nullptr;

    if (TabID == OOT_GAME)
    {   // OoT

        activeTab = this->OoTTab;
    }
    else
    {   // MM

        activeTab = this->MMTab;
    }

    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[5] = { 0 };

    // Initialize the string with : GameName (
    size_t offset = 0;
    size_t typeLen = strlen(activeTab->TabName);
    memcpy_s(finalName, max_size, activeTab->TabName, typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    // Add the number of found object : GameName (foundObjs / 
    _itoa_s((int)activeTab->FoundObjects, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';
    offset += 3;

    // Add the total number of object : GameName (foundObjs / totObjs)
    _itoa_s(activeTab->TotalObjects, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->TabWidget->setTabText(TabID + 1, finalName);
}


void OoTMMComboTracker::UpdateObjectMapVisibility(bool NewValue)
{
    AppConfig::SetHideCollectedFromMap(NewValue);
    this->OoTTab->UpdateObjectVisibility();
    this->MMTab->UpdateObjectVisibility();
}


void OoTMMComboTracker::UpdateObjectListVisibility(bool NewValue)
{
    AppConfig::SetHideCollectedFromObjectList(NewValue);
    this->OoTTab->UpdateObjectVisibility();
    this->MMTab->UpdateObjectVisibility();
}
*/
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
}


void OoTMMComboTracker::ApplySettings()
{
    switch (this->ROMSettings.Game)
    {
        case ROMGame::oot:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), true);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), false);
            this->ROMSettings.ApplyOoTSettingsToFilter(this->OoTTab->GameMaps->FilterButton);
            break;
        }

        case ROMGame::mm:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), false);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), true);
            this->ROMSettings.ApplyMMSettingsToFilter(this->MMTab->GameMaps->FilterButton);
            break;
        }

        case ROMGame::ootmm:
        default:
        {
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->OoTTab), true);
            this->TabWidget->setTabVisible(this->TabWidget->indexOf(this->MMTab), true);
            this->ROMSettings.ApplySettings(this->OoTTab->GameMaps->FilterButton, this->MMTab->GameMaps->FilterButton);
            break;
        }
    }
}


void OoTMMComboTracker::RefreshTracker()
{
    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
    this->EntTab->RefreshEntranceTab();
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
        {
            offset = this->ROMSettings.LoadFileSettings(&data, offset);
            offset = LoadSceneObjects(&data, offset);
            offset = LoadEntrances(&data, offset);
            break;
        }

        case TrackerVersion::V1_1:
        {
            offset = this->ROMSettings.LoadFileSettings(&data, offset);
            LoadSceneObjects(&data, offset);
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

    // Regex to split strings by location
    QRegularExpression reg("^\\s{2}(.+:(?:\n\\s{4}.*)+)\n*", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(sections[2]);

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
        SceneObjects* gameSceneObj = GetGameSceneObjects(spoilerMap[mapName].second);   // Get the correct game objects

        for (qsizetype i = 1; i < objects.size(); i++)
        {   // Browse all the spoiler scene objects

            QStringList spoilObject = objects[i].split(": ");                           // Left part is object name, right is the item it contains
            spoilObject[0] = spoilObject[0].replace("\n", "");                          // Sometimes the object has a line break, just get rid of it

            size_t len = spoilObject[0].length() + 1;                                   // We need to add 1 for the null terminator
            char* tmpObjName = (char*)malloc(sizeof(char) * len);
            memcpy_s(tmpObjName, len, spoilObject[0].toStdString().c_str(), len);
            tmpObjName[len - 1] = '\0';

            GameLayout sceneActiveLayout = GetSceneMetaInfo(sceneID, spoilerMap[mapName].second)->ActiveLayout;

            for (size_t j = 0; j < gameSceneObj[sceneID].NumOfObjs; j++)
            {   // Browse all scenes objects
                
                // Some layout have the same object name but different object position. We need to check if the active layout match the obejct one in order to fill the right one)
                if (gameSceneObj[sceneID].Objects[j].Layout == GameLayout::all || gameSceneObj[sceneID].Objects[j].Layout == sceneActiveLayout)
                {   // The layout does match

                    if (strcmp(gameSceneObj[sceneID].Objects[j].Location, tmpObjName) == 0)
                    {   // We have found the object

                        ObjectInfo* object = &gameSceneObj[sceneID].Objects[j];

                        // Find and modify the object item
                        const ItemInfo* item = FindItemByName(spoilObject[1]);
                        object->Item = item;

                        if (object->RenderScene != sceneID)
                        {   // The current object will never be rendered, we need to update its counter part

                            for (size_t k = 0; k < gameSceneObj[object->RenderScene].NumOfObjs; k++)
                            {   // Find the object in the rendered scene

                                if (strcmp(gameSceneObj[object->RenderScene].Objects[k].Location, object->Location) == 0)
                                {   // Object found

                                    gameSceneObj[object->RenderScene].Objects[k].Item = item;
                                    break;
                                }
                            }
                        }
                        if (object->Type == ObjectType::none)
                        {   // The object is in the good renderer however in some cases it might be not rendered (e.g. MM Mountain village Spring / Winter)

                            continue;   // We should do another loop and not break in order to find the real rendered item
                        }

                        free(tmpObjName);
                        break;
                    }
                }
            }
        }
    }

    this->ApplySettings();
    this->RefreshTracker();

    AppConfig::SetLastSpoilerLogPath(FilePath);
}

#pragma endregion
