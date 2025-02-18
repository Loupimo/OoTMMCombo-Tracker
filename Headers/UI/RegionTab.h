#pragma once

#include <QTreeWidget>
#include "Combo/Regions.h"

class GameTab;

class RegionTree : public QTreeWidgetItem
{

public:

	const RegionMetaInfo* MetaInfo;
	int FoundObjs = 0;					// The total number of objects found in this region
	int TotalObjs = 0;					// The total number of objects this region contains

	GameTab* GameOwner;

	RegionTree(GameTab * Owner, int Game, uint8_t Region, QTreeWidget * Parent);
	~RegionTree();

	void AddObjectCounts(int FoundObjects, int TotalObjects);
	void RefreshObjsCountText();
};