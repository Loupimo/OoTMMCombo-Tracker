#pragma once

#include <QTreeWidget>
#include "Combo/Regions.h"

class GameTab;

/*
* This class is handling the main region data.
*/
class RegionTree : public QTreeWidgetItem
{

public:

	const RegionMetaInfo* MetaInfo;		// The region meta information.
	int FoundObjs = 0;					// The total number of objects found in this region.
	int TotalObjs = 0;					// The total number of objects this region contains.

	GameTab* GameOwner;					// The game tab that owns this region.

public:

	/*
	*   Construct a region tree item for the desired game.
	*
	*   @param Owner            The game that owns this region.
	*   @param Region           The region ID to create.
	*   @param Parent           The parent tree item to attach this item to.
	*/
	RegionTree(GameTab * Owner, uint8_t Region, QTreeWidget * Parent);

	/*
	*   Default destructor.
	*/
	~RegionTree() {}

	/*
	*   Adds the given number of objects to this region counters and update that game tab counters.
	*
	*   @param FoundObjects     The found objects to add to this region.
	*   @param TotalObjects     The total number of objects to add to this region.
	*/
	void AddObjectCounts(int FoundObjects, int TotalObjects);

	/*
	*   Refresh the region text name.
	*/
	void RefreshObjsCountText();
};