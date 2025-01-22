#include "UI/ObjectRenderer.h"

ObjectRenderer::ObjectRenderer(ObjectType Type)
{
    // Charger l'image à afficher
    this->Icon = QPixmap(Icons[Type].IconPath);
    this->Icon = this->Icon.scaled(Icons[Type].Scale[0], Icons[Type].Scale[1], Qt::KeepAspectRatio);

    // Liste des positions où afficher les images
    /*points = {{353, 501}, {607, 493}, {455, 493}, {517, 488},
{310, 473}, {765, 473}, {697, 473}, {635, 473},
{181, 472}, {248, 471}, {533, 454}, {474, 454},
{420, 453}, {472, 362}, {429, 356}, {318, 341},
{192, 337}, {572, 337}, {484, 337}, {525, 335},
{364, 335}, {278, 335}, {411, 329}, {564, 311},
{354, 303}, {475, 301}, {435, 301}, {310, 299},
{611, 297}, {520, 295}, {570, 283}, {375, 274},
{425, 273}, {476, 268}, {476, 174}, {433, 171},
{524, 166}, {568, 164}, {476, 146}, {519, 141},
{574, 136}, {432, 135}, {524, 114}, {460, 114} };
    */

    /*points = {

       {997, 296}, {918, 611}

    };*/
}

void ObjectRenderer::AddObjectToRender(int X, int Y)
{
    X = X - (Icon.width() / 2);

    if (X < 0)
    {
        X = 0;
    }

    Y = Y - (Icon.height() / 2);

    if (Y < 0)
    {
        Y = 0;
    }

    this->ObjectsLocation.append(std::pair<int, int>(X, Y));
}



void ObjectRenderer::AddObjectToScene(QGraphicsScene* Destination)
{
    if (this->ShouldBeRendered)
    {
        for (const auto& point : this->ObjectsLocation)
        {
            QGraphicsPixmapItem* iconItem = new QGraphicsPixmapItem(this->Icon);
            iconItem->setPos(point.first, point.second); // Positionner les images
            Destination->addItem(iconItem);
        }
    }
}