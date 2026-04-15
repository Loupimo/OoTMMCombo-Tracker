#pragma array_limit 1000000

struct entranceLink
{
    u32 inLink;
    u32 outLink;
    u8 inLinkGame;
    u8 outLinkGame;
};

struct entranceMap
{
    u32 entranceID;
    //u64 numOfObjs;
    entranceLink link;
};

struct entranceMetaInf
{
    u32 sceneID;
    u64 numOfObjs;
    entranceMap ent[numOfObjs];
};

struct entrance
{
    u64 numOfEnts;
    entranceMetaInf ents[numOfEnts];
};

struct object
{
    u32 objectID;
    u32 state;
    u32 itemID;
};

struct scene
{
    u32 sceneID;
    u64 numObjs;
    object objects[numObjs];
};

struct settings
{
    u32 version;
    u64 numOfParams;
    //u8 params[numOfParams];
};

struct trck
{
    settings set;
    scene oot[153];
    scene mm[152];
    entrance ootEnt;
    entrance mmEnt;
};

trck tracking @ 0x00;
//scene oot[147] @ 0x00 + sizeof (sets);

//scene mm[143] @ 0x00 + sizeof(oot) + sizeof (sets);