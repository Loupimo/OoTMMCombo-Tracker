#pragma once

#include <QWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPair>

enum ObjectType
{
	none = 0,
	chest = 1,
	collectible = 2,
	npc = 3,
	gs = 4,			// Gold skulltula
	sf = 5,			// Stray fairy
	cow = 6,
	shop = 7,
	scrub = 8,
	sr = 9,			// Silver Rupee
	fish = 10,
	wonder = 11,
	grass = 12,
	crate = 13,
	pot = 14,
	hive = 15,
	butterfly = 16,
	rupee = 17,
	snowball = 18,
	barrel = 19,
	heart = 20,
	fairy_spot = 21,
	fairy = 22,
};

class ObjectRenderer : public QGraphicsPixmapItem
{
public:
	ObjectRenderer(ObjectType Type);
	void AddRendererToScene(QGraphicsScene* Destination);

private:
	QPixmap Icon;                        // Image à afficher
	QList<QPair<int, int>> points; // Liste des positions (X, Y)
};