#include "UI/GameTab.h"
#include "Multi/Game.h"


MainRegionTab::MainRegionTab()
{
    this->addTab(&this->Overworld, "Overworld");
    this->addTab(&this->Temples, "Temples");
    this->addTab(&this->Houses, "Houses");
    this->addTab(&this->Grottos, "Grottos");

    connect(this, &QTabWidget::currentChanged, this, &MainRegionTab::LoadTab);
}

MainRegionTab::~MainRegionTab()
{}


void MainRegionTab::LoadTab(int TabIndex)
{
    this->UnloadTab(this->PrevTab);
    switch (TabIndex)
    {
        case 0:
        {
            this->Overworld.RenderTab();
            break;
        }
        case 1:
        {
            this->Temples.RenderTab();
            break;
        }
        case 2:
        {
            this->Houses.RenderTab();
            break;
        }
        case 3:
        {
            this->Grottos.RenderTab();
            break;
        }
    }

    this->PrevTab = TabIndex;
}


void MainRegionTab::UnloadTab(int TabIndex)
{
    switch (TabIndex)
    {
        case 0:
        {
            this->Overworld.UnloadTab();
            break;
        }
        case 1:
        {
            this->Temples.UnloadTab();
            break;
        }
        case 2:
        {
            this->Houses.UnloadTab();
            break;
        }
        case 3:
        {
            this->Grottos.UnloadTab();
            break;
        }
    }

    this->PrevTab = TabIndex;
}


GameTab::GameTab(int GameID, QWidget* parent) : QWidget(parent)
{
	this->GameID = GameID;
	if (GameID == OOT_GAME)
	{	// Ocarina of time

		this->TabName = "OoT";
	}
	else
	{	// Majora's mask

		this->TabName = "MM";
	}

    // Layout principal
    this->MainLayout = new QHBoxLayout;

    // Zone graphique pour la carte
    //this->View = new QGraphicsView();


    // Ajouter à la mise en page principale
    this->MainLayout->addWidget(&this->MainRegion);
    //this->MainLayout->addWidget(this->View);
    this->setLayout(this->MainLayout);
}


GameTab::~GameTab()
{

}