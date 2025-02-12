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

	QFont font = this->font(0);
	font.setBold(true);
	this->setFont(0, font);
	this->setText(0, this->MetaInfo->RegionName);
	this->setIcon(0, QIcon(this->MetaInfo->Path));
}


RegionTree::~RegionTree()
{
}


void RegionTree::AddObjectCounts(int FoundObjects, int TotalObjects)
{
	this->FoundObjs += FoundObjects;
	this->TotalObjs += TotalObjects;
}


void RegionTree::RefreshObjsCountText()
{
    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[5] = { 0 };

    size_t offset = 0;
    size_t typeLen = strlen(this->MetaInfo->RegionName);
    memcpy_s(finalName, max_size, this->MetaInfo->RegionName, typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    _itoa_s((int)this->FoundObjs, tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);

    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';

    offset += 3;

    _itoa_s((int)this->TotalObjs, tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));

    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->setText(0, finalName);
}