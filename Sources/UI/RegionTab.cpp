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
    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[5] = { 0 };

    // Initialize the string with : RegionName (
    size_t offset = 0;
    size_t typeLen = strlen(this->MetaInfo->RegionName);
    memcpy_s(finalName, max_size, this->MetaInfo->RegionName, typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    // Add the number of found object : RegionName (foundObjs /
    _itoa_s((int)this->FoundObjs, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';
    offset += 3;

    // Add the total number of object : RegionName (foundObjs / totObjs) 
    _itoa_s((int)this->TotalObjs, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->setText(0, finalName);
}