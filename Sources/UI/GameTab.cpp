#include "UI/GameTab.h"
#include "Multi/Game.h"
#include "UI/SceneRenderer.h"

#define CreateOverworldScene(SceneID, Path, Game) SceneInfo (SceneID, Path, Game, SceneType::Overworld)
#define CreateTempleScene(SceneID, Path, Game) SceneInfo (SceneID, Path, Game, SceneType::Temple)
#define CreateHouseScene(SceneID, Path, Game) SceneInfo (SceneID, Path, Game, SceneType::House)
#define CreateGrottoScene(SceneID, Path, Game) SceneInfo (SceneID, Path, Game, SceneType::Grotto)

#define CreateInfoScenes(NumOfScenes, Prefix, ...) \
const size_t Prefix##Size = NumOfScenes;              \
SceneInfo Prefix##Scenes[Prefix##Size] = { __VA_ARGS__ };


CreateInfoScenes(1, OoTOverworld,
    CreateOverworldScene(HYRULE_FIELD, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
)

CreateInfoScenes(1, OoTTemple,
    CreateTempleScene(TEMPLE_FIRE, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME),
)

CreateInfoScenes(1, OoTHouse,
    CreateHouseScene(LINK_HOUSE, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
)

CreateInfoScenes(1, OoTGrotto,
    CreateGrottoScene(OOT_GROTTO_CASTLE_STORMS, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
)



CreateInfoScenes(1, MMOverworld,
    CreateOverworldScene(HYRULE_FIELD, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
)

CreateInfoScenes(1, MMTemple,
    CreateTempleScene(TEMPLE_FIRE, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME),
    )

    CreateInfoScenes(1, MMHouse,
        CreateHouseScene(LINK_HOUSE, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
    )

    CreateInfoScenes(1, MMGrotto,
        CreateGrottoScene(OOT_GROTTO_CASTLE_STORMS, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
    )



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