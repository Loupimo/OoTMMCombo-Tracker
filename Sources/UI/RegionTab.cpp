#include "UI/RegionTab.h"
#include "Multi/Game.h"

RegionTree::RegionTree(int Game, uint8_t Region, QTreeWidget * Parent) : QTreeWidgetItem(Parent)
{
	if (Game == OOT_GAME)
	{
		this->MetaInfo = &OoTRegionsMetaInfo[Region];
	}
	else
	{
		this->MetaInfo = &MMRegionsMetaInfo[Region];
	}

	this->setText(0, this->MetaInfo->RegionName);
}

RegionTree::~RegionTree()
{
}