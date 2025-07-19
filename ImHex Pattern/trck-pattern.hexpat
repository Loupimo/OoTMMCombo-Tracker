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
scene oot[147] @ 0x00;

scene mm[143] @ 0x00 + sizeof(oot);