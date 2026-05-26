#include "UI/SceneEntrance.h"
#include "UI/AppConfig.h"


void EntranceLink::SaveLink(QFile* SaveFile)
{
    QByteArray tmpU32(sizeof(uint32_t), 0);
    QByteArray tmpU8(sizeof(uint8_t), 0);

    // Save Out link ID
    memcpy_s(tmpU32.data(), sizeof(uint32_t), &this->OutLink, sizeof(this->OutLink));
    SaveFile->write(tmpU32);

    // Save Out link Game
    memcpy_s(tmpU8.data(), sizeof(uint8_t), &this->OutLinkGame, sizeof(this->OutLinkGame));
    SaveFile->write(tmpU8);

    // Save the number of inbound sources, then each (EntranceID, Game) pair
    uint32_t numInLinks = (uint32_t)this->InLinks.size();
    memcpy_s(tmpU32.data(), sizeof(uint32_t), &numInLinks, sizeof(numInLinks));
    SaveFile->write(tmpU32);

    for (const EntranceSource& src : this->InLinks)
    {
        memcpy_s(tmpU32.data(), sizeof(uint32_t), &src.EntranceID, sizeof(src.EntranceID));
        SaveFile->write(tmpU32);

        memcpy_s(tmpU8.data(), sizeof(uint8_t), &src.Game, sizeof(src.Game));
        SaveFile->write(tmpU8);
    }
}

size_t EntranceLink::LoadLink(QByteArray* Data, size_t Offset, TrackerVersion Version)
{
    this->InLinks.clear();

    if (Version == TrackerVersion::V2_0)
    {
        // Legacy 10-byte layout: InLink (u32), OutLink (u32), InLinkGame (u8), OutLinkGame (u8).
        // A single-source InLink is promoted to a one-entry list; an undiscovered InLink
        // (UINT32_MAX) yields an empty list, matching the new model's "no source" semantics.
        uint32_t legacyInLink = UINT32_MAX;
        memcpy_s(&legacyInLink, sizeof(legacyInLink), Data->data() + Offset, sizeof(legacyInLink));
        Offset += sizeof(legacyInLink);

        memcpy_s(&this->OutLink, sizeof(this->OutLink), Data->data() + Offset, sizeof(this->OutLink));
        Offset += sizeof(this->OutLink);

        uint8_t legacyInLinkGame = NO_GAME;
        memcpy_s(&legacyInLinkGame, sizeof(legacyInLinkGame), Data->data() + Offset, sizeof(legacyInLinkGame));
        Offset += sizeof(legacyInLinkGame);

        memcpy_s(&this->OutLinkGame, sizeof(this->OutLinkGame), Data->data() + Offset, sizeof(this->OutLinkGame));
        Offset += sizeof(this->OutLinkGame);

        if (legacyInLink != UINT32_MAX && legacyInLinkGame != NO_GAME)
        {
            this->InLinks.push_back({ legacyInLink, legacyInLinkGame });
        }

        return Offset;
    }

    // V2_1 layout: OutLink (u32), OutLinkGame (u8), count (u32), then count * (EntranceID u32, Game u8).
    memcpy_s(&this->OutLink, sizeof(this->OutLink), Data->data() + Offset, sizeof(this->OutLink));
    Offset += sizeof(this->OutLink);

    memcpy_s(&this->OutLinkGame, sizeof(this->OutLinkGame), Data->data() + Offset, sizeof(this->OutLinkGame));
    Offset += sizeof(this->OutLinkGame);

    uint32_t numInLinks = 0;
    memcpy_s(&numInLinks, sizeof(numInLinks), Data->data() + Offset, sizeof(numInLinks));
    Offset += sizeof(numInLinks);

    this->InLinks.reserve(numInLinks);
    for (uint32_t i = 0; i < numInLinks; i++)
    {
        EntranceSource src;
        memcpy_s(&src.EntranceID, sizeof(src.EntranceID), Data->data() + Offset, sizeof(src.EntranceID));
        Offset += sizeof(src.EntranceID);

        memcpy_s(&src.Game, sizeof(src.Game), Data->data() + Offset, sizeof(src.Game));
        Offset += sizeof(src.Game);

        this->InLinks.push_back(src);
    }

    return Offset;
}


void EntranceLink::ResetLink()
{
    this->InLinks.clear();
    this->OutLink = UINT32_MAX;
    this->OutLinkGame = NO_GAME;
}


void EntranceLink::AddInLink(uint32_t EntranceID, uint8_t Game)
{
    // Dedup: traversing the same path twice should not duplicate the source.
    for (const EntranceSource& src : this->InLinks)
    {
        if (src.EntranceID == EntranceID && src.Game == Game)
        {
            return;
        }
    }
    this->InLinks.push_back({ EntranceID, Game });
}


void SceneEntranceMetaInf::SaveMetaInf(QFile* SaveFile)
{
    QByteArray tmp(sizeof(uint32_t), 0);
    QByteArray numObj(sizeof(size_t), 0);

    // Save Scene ID
    memcpy_s(tmp.data(), 4, &this->SceneID, sizeof(this->SceneID));
    SaveFile->write(tmp);

    // Save number of entrance links
    size_t numObjs = this->EntranceIDs.size();
    memcpy_s(numObj.data(), sizeof(size_t), &numObjs, sizeof(numObjs));
    SaveFile->write(numObj);

    for (auto& [entranceID, entrance] : this->EntranceIDs)
    {	// Save all entrance links

        // Save entrance ID
        memcpy_s(tmp.data(), 4, &entranceID, sizeof(entranceID));
        SaveFile->write(tmp);

        // Save entrance Link
        entrance.SaveLink(SaveFile);
    }
}

size_t SceneEntranceMetaInf::LoadMetaInf(QByteArray* Data, size_t Offset, TrackerVersion Version)
{
    // Load scene ID
    uint32_t sceneID = 0;
    memcpy_s(&sceneID, sizeof(sceneID), Data->data() + Offset, sizeof(sceneID));
    Offset += sizeof(sceneID);

    if (this->SceneID == sceneID)
    {	// The scene is correct

        // Load number of objects
        size_t numObjs = 0;
        memcpy_s(&numObjs, sizeof(numObjs), Data->data() + Offset, sizeof(numObjs));
        Offset += sizeof(numObjs);

        if (numObjs == this->EntranceIDs.size())
        {	// It has the same number of objects

            for (auto& [currEntranceID, entrance] : this->EntranceIDs)
            {	// Load all entrance links

                // Load entrance ID
                uint32_t entranceID = 0;
                memcpy_s(&entranceID, sizeof(entranceID), Data->data() + Offset, sizeof(entranceID));
                Offset += sizeof(entranceID);

                if (entranceID == currEntranceID)
                {
                    Offset = entrance.LoadLink(Data, Offset, Version);
                }
            }
        }
        else
        {
            Offset += numObjs * (sizeof(uint32_t) * 3 + sizeof(bool) * 2);
        }
    }

    return Offset;
}


void SceneEntranceMetaInf::ResetMetaInf()
{
    for (auto& [currEntranceID, entrance] : this->EntranceIDs)
    {	// Load all entrance links

        entrance.ResetLink();
    }
}


SceneEntranceMetaInf * GetSceneEntranceMetaInf(int Game, uint32_t SceneID)
{
	if (Game == OOT_GAME)
	{
		if (OoTSceneEntranceMeta.contains(SceneID))
		{
			return &OoTSceneEntranceMeta.at(SceneID);
		}
	}
	else if (Game == MM_GAME)
	{
		if (MMSceneEntranceMeta.contains(SceneID))
		{
			return &MMSceneEntranceMeta.at(SceneID);
		}
	}

	return NULL;
}


std::map<uint32_t, SceneEntranceMetaInf>* GetSceneEntranceMetaInfForGame(int Game)
{
	if (Game == OOT_GAME)
	{
		return &OoTSceneEntranceMeta;
	}
	else if (Game == MM_GAME)
	{
		return &MMSceneEntranceMeta;
	}

	return NULL;
}


void SaveEntrances(QFile* SaveFile)
{
    SaveEntrancesFor(SaveFile, GetSceneEntranceMetaInfForGame(OOT_GAME));
    SaveEntrancesFor(SaveFile, GetSceneEntranceMetaInfForGame(MM_GAME));
}


void SaveEntrancesFor(QFile* SaveFile, std::map<uint32_t, SceneEntranceMetaInf>* Array)
{
    QByteArray numObj(sizeof(size_t), 0);

    // Save number of scene entrances
    size_t numOfObjs = Array->size();
    memcpy_s(numObj.data(), sizeof(numOfObjs), &numOfObjs, sizeof(numOfObjs));
    SaveFile->write(numObj);

    for (auto& [sceneID, scene] : *Array)
    {	// Save all scenes meta info

        scene.SaveMetaInf(SaveFile);
    }
}


size_t LoadEntrances(QByteArray* Data, size_t Offset, TrackerVersion Version)
{
    Offset = LoadEntrancesFor(Data, Offset, GetSceneEntranceMetaInfForGame(OOT_GAME), Version);
    Offset = LoadEntrancesFor(Data, Offset, GetSceneEntranceMetaInfForGame(MM_GAME), Version);

    return Offset;
}


size_t LoadEntrancesFor(QByteArray* Data, size_t Offset, std::map<uint32_t, SceneEntranceMetaInf>* Array, TrackerVersion Version)
{
    QByteArray numObj(sizeof(size_t), 0);

    // Load number of scene entrances
    size_t numOfObjs = 0;
    memcpy_s(&numOfObjs, sizeof(numOfObjs),  Data->data() + Offset, sizeof(numOfObjs));
    Offset += sizeof(numOfObjs);
    //int i = Array->size();
    if (numOfObjs != Array->size())
    {
        return -1;
    }

    for (auto& [sceneID, scene] : *Array)
    {	// Save all scenes meta info

        if (sceneID == scene.SceneID)
        {
            Offset = scene.LoadMetaInf(Data, Offset, Version);
        }
    }

    return Offset;
}


void ResetEntrancesFor(std::map<uint32_t, SceneEntranceMetaInf>* Array)
{
    for (auto& [sceneID, scene] : *Array)
    {	// Reset all scenes meta info

        scene.ResetMetaInf();
    }
}


void ResetAllEntrances()
{
    ResetEntrancesFor(GetSceneEntranceMetaInfForGame(OOT_GAME));
    ResetEntrancesFor(GetSceneEntranceMetaInfForGame(MM_GAME));
}
