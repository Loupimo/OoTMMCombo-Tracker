#pragma array_limit 1000000

struct link
{
    u32 linkID;
    u8 linkGame;
};

struct entranceLink
{
    link outLink;
    u32 numOfInLink;
    link inLinks[numOfInLink];
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
    u64 numOfEntrances;
    //entranceMap ent;
    entranceMap ent[numOfEntrances];
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
    u32 worldID;
};

struct scene
{
    u32 sceneID;
    u64 numObjs;
    object objects[numObjs];
};

struct world
{
    scene oot[153];
    scene mm[154];
};

struct settings
{
    u32 version;
    u64 numOfParams;
    u32 numOfWorlds;
    world Worlds[numOfWorlds];
    //u8 params[numOfParams];
};

struct trck
{
    settings set;
    entrance ootEnt;
    entrance mmEnt;
};

trck tracking @ 0x00;
//scene oot[147] @ 0x00 + sizeof (sets);

//scene mm[143] @ 0x00 + sizeof(oot) + sizeof (sets);