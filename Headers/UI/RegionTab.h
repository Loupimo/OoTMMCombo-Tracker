#pragma once

#include <QTreeWidget>
#include "Combo/Regions.h"

class RegionTree : public QTreeWidgetItem
{

public:

	const RegionMetaInfo* MetaInfo;

	RegionTree(int Game, uint8_t Region, QTreeWidget * Parent);
	~RegionTree();
};