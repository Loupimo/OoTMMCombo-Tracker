#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include "UI/MapTab.h"

// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(const QString& MapPath, QWidget* parent) : QWidget(parent)
{
    // Layout principal
    this->MainLayout = new QHBoxLayout;

    // Zone graphique pour la carte
    this->Scene = new QGraphicsScene;
    this->View = new QGraphicsView(this->Scene);

    // Charger l'image de la carte
    this->Map = new QPixmap(MapPath);
    this->Scene->addPixmap(*this->Map);

    // Ajouter des points cliquables
    //QGraphicsEllipseItem* point = this->Scene->addEllipse(50, 50, 10, 10, QPen(Qt::red), QBrush(Qt::red));
    this->Objects.push_back(new ObjectRenderer(ObjectType::pot));
    this->Objects[0]->AddRendererToScene(this->Scene);
    //this->Scene->addWidget(this->Objects[0]);

    // Panneau latéral
    QVBoxLayout* sidePanelLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("Informations sur les éléments");
    QPushButton* button = new QPushButton("Action");
    sidePanelLayout->addWidget(infoLabel);
    sidePanelLayout->addWidget(button);
    sidePanelLayout->addStretch();

    QWidget* sidePanel = new QWidget;
    sidePanel->setLayout(sidePanelLayout);

    // Ajouter à la mise en page principale
    this->MainLayout->addWidget(this->View);
    this->MainLayout->addWidget(sidePanel);

    this->setLayout(this->MainLayout);
}

MapTab::~MapTab()
{
    this->View->~QGraphicsView();
    this->Scene->~QGraphicsScene();
    this->Map->~QPixmap();
    this->MainLayout->~QHBoxLayout();
}