#include "UI/RegionTab.h"
#include "Multi/Game.h"
#include "UI/GameTab.h"

RegionTree::RegionTree(GameTab* Owner, uint8_t Region, QTreeWidget * Parent) : QTreeWidgetItem(Parent)
{
    this->GameOwner = Owner;
	if (this->GameOwner->GameID == OOT_GAME)
	{
		this->MetaInfo = &OoTRegionsMetaInfo[Region];
	}
	else
	{
		this->MetaInfo = &MMRegionsMetaInfo[Region];
	}

	QFont font = this->font(0);
	font.setBold(true);
	this->setFont(0, font);
	this->setText(0, this->MetaInfo->RegionName);
	this->setIcon(0, QIcon(this->MetaInfo->Path));
}


void RegionTree::AddObjectCounts(int FoundObjects, int TotalObjects)
{
	this->FoundObjs += FoundObjects;
	this->TotalObjs += TotalObjects;
    this->GameOwner->FoundObjects += FoundObjects;
    this->GameOwner->TotalObjects += TotalObjects;
    this->GameOwner->RefreshTabCountText();
}


void RegionTree::RefreshObjsCountText()
{
    QString finalName = BuildCountLabel(this->MetaInfo->RegionName, this->FoundObjs, this->TotalObjs);
    this->setText(0, finalName);
}


void RegionTree::ResetRegion()
{
    this->FoundObjs = 0;
    this->TotalObjs = 0;
}


RegionTree* FindRegionTreeIn(const std::vector<RegionTree*>& Regions, uint8_t Region)
{
    for (size_t i = 0; i < Regions.size(); i++)
    {   // Browse through all the available regions

        if (Regions[i]->MetaInfo->Region == Region)
        {   // We have found the matching region

            return Regions[i];
        }
    }

    return nullptr;
}