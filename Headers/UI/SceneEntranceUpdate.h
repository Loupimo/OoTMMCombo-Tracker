#pragma once

#include <cstdint>
#include <QMetaType>


/*
*   Forward declaration. The full definition lives in SceneEntrance.h.
*   SceneEntranceUpdate only stores a pointer to an EntranceLink (which is
*   owned by the global OoTSceneEntranceMeta / MMSceneEntranceMeta maps),
*   so a forward declaration is enough here and prevents include cycles
*   between Multi.h and the entrance headers.
*/
struct EntranceLink;


/*
*   Snapshot of a single entrance link update emitted by the EntranceHelper
*   to the UI thread. Designed to be passed by value through Qt queued
*   signals so the receiver always sees a valid copy, even after the
*   sender's local variables have been destroyed.
*/
typedef struct SceneEntranceUpdate
{
    uint8_t Game;
    uint32_t SceneID;
    uint32_t EntranceID;
    EntranceLink* Link;
} SceneEntranceUpdate;


Q_DECLARE_METATYPE(SceneEntranceUpdate)
