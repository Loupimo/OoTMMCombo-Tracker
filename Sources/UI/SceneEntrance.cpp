#include "UI/SceneEntrance.h"


void EntranceLink::SaveLink(QFile* SaveFile)
{
    QByteArray tmp(sizeof(uint32_t), 0);
    QByteArray tmpGame(sizeof(uint8_t), 0);

    // Save In link ID
    memcpy_s(tmp.data(), 4, &this->InLink, sizeof(this->InLink));
    SaveFile->write(tmp);

    // Save Out link ID
    memcpy_s(tmp.data(), 4, &this->OutLink, sizeof(this->OutLink));
    SaveFile->write(tmp);

    // Save In link Game
    memcpy_s(tmpGame.data(), 1, &this->InLinkGame, sizeof(this->InLinkGame));
    SaveFile->write(tmpGame);

    // Save Out link Game
    memcpy_s(tmpGame.data(), 1, &this->OutLinkGame, sizeof(this->OutLinkGame));
    SaveFile->write(tmpGame);
}

size_t EntranceLink::LoadLink(QByteArray* Data, size_t Offset)
{
    // Load In link ID
    memcpy_s(&this->InLink, sizeof(this->InLink), Data->data() + Offset, sizeof(this->InLink));
    Offset += sizeof(this->InLink);

    // Load Out link ID
    memcpy_s(&this->OutLink, sizeof(this->OutLink), Data->data() + Offset, sizeof(this->OutLink));
    Offset += sizeof(this->OutLink);

    // Load In link Game
    memcpy_s(&this->InLinkGame, sizeof(this->InLinkGame), Data->data() + Offset, sizeof(this->InLinkGame));
    Offset += sizeof(this->InLinkGame);

    // Load Out link Game
    memcpy_s(&this->OutLinkGame, sizeof(this->OutLinkGame), Data->data() + Offset, sizeof(this->OutLinkGame));
    Offset += sizeof(this->OutLinkGame);

    return Offset;
}


void EntranceLink::ResetLink()
{
    this->InLink = UINT32_MAX;
    this->OutLink = UINT32_MAX;
    this->InLinkGame = NO_GAME;
    this->OutLinkGame = NO_GAME;
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

size_t SceneEntranceMetaInf::LoadMetaInf(QByteArray* Data, size_t Offset)
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
                    Offset = entrance.LoadLink(Data, Offset);
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


size_t LoadEntrances(QByteArray* Data, size_t Offset)
{
    Offset = LoadEntrancesFor(Data, Offset, GetSceneEntranceMetaInfForGame(OOT_GAME));
    Offset = LoadEntrancesFor(Data, Offset, GetSceneEntranceMetaInfForGame(MM_GAME));

    return Offset;
}


size_t LoadEntrancesFor(QByteArray* Data, size_t Offset, std::map<uint32_t, SceneEntranceMetaInf>* Array)
{
    QByteArray numObj(sizeof(size_t), 0);

    // Load number of scene entrances
    size_t numOfObjs = 0;
    memcpy_s(&numOfObjs, sizeof(numOfObjs),  Data->data() + Offset, sizeof(numOfObjs));
    Offset += sizeof(numOfObjs);
    int i = Array->size();
    if (numOfObjs != Array->size())
    {
        return -1;
    }

    for (auto& [sceneID, scene] : *Array)
    {	// Save all scenes meta info

        if (sceneID == scene.SceneID)
        {
            Offset = scene.LoadMetaInf(Data, Offset);
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
