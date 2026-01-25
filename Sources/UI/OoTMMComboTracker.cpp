#include "Combo/Objects.h"
#include "UI/AppConfig.h"
#include "UI/OoTMMComboTracker.h"
#include <QMessageBox>
#include <QFileDialog>

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
    
    // Add the tabs to the widget : Log -> OoT -> MM
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(this->OoTTab, this->OoTTab->TabName);
    this->TabWidget->addTab(this->MMTab, this->MMTab->TabName);

    // Update game tabs name
    this->UpdateTabNameText(0);
    this->UpdateTabNameText(1);
    this->setCentralWidget(this->TabWidget);
    this->setWindowTitle("OoTMMCombo Auto Tracker");
    this->setWindowIcon(QIcon("./Resources/Logo.ico"));

    // Loads default value
    this->ui.actionAutoSnapView->setChecked(AppConfig::GetAutoSnapView());
    this->ui.actionAutoZoom->setChecked(AppConfig::GetAutoZoom());
    this->ui.actionAutoSaving->setChecked(AppConfig::GetAutoSave());
    this->ui.actionAutoLoadTrackingFile->setChecked(AppConfig::GetAutoLoadTrackingFile());
    this->ui.actionAutoLoadSpoilerLog->setChecked(AppConfig::GetAutoLoadSpoilerLog());
    this->ui.actionHideCollectedObject->setChecked(AppConfig::GetHideCollectedObject());

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
    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
    connect(this->ui.actionSaveSession, &QAction::triggered, this->Log, &LogTab::SaveTracking);
    connect(this->ui.actionLoadSession, &QAction::triggered, this->Log, &LogTab::LoadTracking);
    connect(this->ui.actionLoadSpoilerLog, &QAction::triggered, this->Log, &LogTab::LoadSpoiler);
    connect(this->ui.actionResetTracking, &QAction::triggered, this->Log, &LogTab::ResetTracking);
    connect(this->ui.actionStartTracking, &QAction::triggered, this->Log, &LogTab::PressLaunchButton);
    connect(this->ui.actionAutoSnapView, &QAction::toggled, this, &AppConfig::SetAutoSnapView);
    connect(this->ui.actionAutoZoom, &QAction::toggled, this, &AppConfig::SetAutoZoom);
    connect(this->ui.actionHideCollectedObject, &QAction::toggled, this, &OoTMMComboTracker::UpdateObjectVisibility);
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
    QMessageBox::about(this, "About", "OoTMMCombo Tracker\nVersion 1.0\n(c) 2025 Loupimo");
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
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath());
            }
            break;
        }
        case MM_GAME:
        {
            this->MMTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath());
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

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


void OoTMMComboTracker::UpdateObjectVisibility(bool NewValue)
{
    AppConfig::SetHideCollectedObject(NewValue);
    this->OoTTab->UpdateObjectVisibility();
    this->MMTab->UpdateObjectVisibility();
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
    
    int numRecentFiles = min(this->RecentFiles.size(), this->MaxRecentFiles);

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


void OoTMMComboTracker::RefreshTracker()
{
    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
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

    LoadSceneObjects(&data, 0);
    loadFile.close();

    MultiLogger::LogMessage("File loaded: %s\n", FilePath.toStdString().c_str());

    this->RefreshTracker();
}


void OoTMMComboTracker::LoadGameSpoiler(QString FilePath)
{
    QFile file(FilePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Can't open file:" << file.errorString();
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
        qWarning() << "The file does not have the correct number of sections.";
        return;
    }

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

            for (size_t j = 0; j < gameSceneObj[sceneID].NumOfObjs; j++)
            {   // Browse all scenes objects

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

    this->RefreshTracker();

    AppConfig::SetLastSpoilerLogPath(FilePath);
}

#pragma endregion