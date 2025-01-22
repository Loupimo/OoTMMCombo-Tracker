#include "UI/LogTab.h"

LogTab::LogTab(QWidget* parent) : QWidget(parent)
{
    this->IsRunning = false;
    this->EnableMultiplayer = false;

    // Conteneur principal encadré
    this->LaunchGroup = new QGroupBox("Launch Options");

    // Bouton avec texte
    this->LaunchButton = new QPushButton("Start Tracking");
    QObject::connect(this->LaunchButton, &QPushButton::pressed, this, &LogTab::PressLaunchButton);

    // Checkbox avec texte à droite
    this->NetCheckBox = new QCheckBox("Use Multiplayer");
    this->NetCheckBox->setChecked(false);
    QObject::connect(this->NetCheckBox, &QCheckBox::checkStateChanged, this, &LogTab::ToggleNetOption);

    // Champ pour URL
    this->Host = new QLineEdit;
    this->Host->setPlaceholderText("Enter the host server address");
    this->Host->setToolTip("Example : multi.ootmm.com");
    this->Host->setText("multi.ootmm.com");
    this->Host->setEnabled(this->EnableMultiplayer);

    // Champ pour un entier
    this->Port = new QLineEdit;
    this->Port->setPlaceholderText("Enter the port to use");
    this->Port->setToolTip("Integer between 0 et 65535. Default: 13248");
    this->Port->setText("13248");
    this->Port->setEnabled(this->EnableMultiplayer);

    // Ajout de validations
    QValidator* intValidator = new QIntValidator(0, 65535); // Valide les entiers
    this->Port->setValidator(intValidator);

    // Layout pour aligner les widgets horizontalement
    this->MultiLayout = new QHBoxLayout;
    this->MultiLayout->addWidget(this->NetCheckBox);   // Ajouter la checkbox
    this->MultiLayout->addWidget(this->Host);  // Ajouter le champ URL
    this->MultiLayout->addWidget(this->Port);  // Ajouter le champ Port


    // Layout pour aligner les widgets verticalement
    this->NetLayout = new QVBoxLayout;
    this->NetLayout->addLayout(this->MultiLayout);   // Ajouter la checkbox
    this->NetLayout->addWidget(this->LaunchButton);  // Ajouter le bouton


    // Assigner le layout au conteneur encadré
    this->LaunchGroup->setLayout(this->NetLayout);

    // Layout principal pour la fenêtre
    this->MainLayout = new QVBoxLayout;
    this->MainLayout->addWidget(this->LaunchGroup);

	this->LogViewer = new QPlainTextEdit;
    this->LogViewer->setReadOnly(true);
    this->MainLayout->addWidget(this->LogViewer);
	this->setLayout(this->MainLayout);

    // Init other attribute
    this->Tracker = new App();
    connect(MultiLogger::GetLogger(), &MultiLogger::LogMsgToView, this, &LogTab::LogMessage);
}


LogTab::~LogTab()
{
    if (this->Tracker && this->Tracker->IsRunning)
    {
        this->Tracker->IsRunning = false;
        this->TrackerThread.join();
        this->Tracker->~App();
    }
    this->LaunchButton->~QPushButton();
    this->NetCheckBox->~QCheckBox();
    this->Host->~QLineEdit();
    this->Port->~QLineEdit();
    this->MultiLayout->~QHBoxLayout();
    this->NetLayout->~QVBoxLayout();
    this->LaunchGroup->~QGroupBox();
    this->LogViewer->~QPlainTextEdit();
    this->MainLayout->~QVBoxLayout();
}


void LogTab::ToggleNetOption(int state)
{
    if (state == Qt::Checked)
    {
        this->EnableMultiplayer = true;
    }
    else
    {
        this->EnableMultiplayer = false;
    }
    this->Host->setEnabled(this->EnableMultiplayer);
    this->Port->setEnabled(this->EnableMultiplayer);
}

void LogTab::PressLaunchButton()
{
    if (this->Tracker)
    {
        if (this->Tracker->IsRunning)
        {   // Stop the auto-tracker

            this->Tracker->IsRunning = false;
            this->LaunchButton->setText("Start Tracking");
            this->TrackerThread.join();
            this->Tracker->appQuit();
        }
        else
        {   // Start the auto-tracker

            if (this->Tracker->appInit())
                return;
            if (this->Tracker->appStartPj64("localhost", 13249))
            {
                this->Tracker->appQuit();
                return;
            }
            if (this->Tracker->appStartAres("localhost", 9123))
            {
                this->Tracker->appQuit();
                return;
            }
            this->LaunchButton->setText("Stop Tracking");
            this->Tracker->IsRunning = true;

            this->TrackerThread = std::thread(&App::appRun, this->Tracker, this->EnableMultiplayer, this->Host, this->Port->text().toUShort());
        }
    }
}

void LogTab::LogMessage(const QString& Message)
{
    this->LogViewer->appendPlainText(Message);
    this->LogViewer->verticalScrollBar()->setValue(this->LogViewer->verticalScrollBar()->maximum());
}