#pragma once

#include "gate.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LG_MAX_GATES 64
#define LG_MAX_OBJECTIVES 6
#define LG_MAX_EDGES (LG_MAX_GATES * LG_MAX_INPUTS)

typedef enum LevelId {
    LEVEL_1 = 0,
    LEVEL_2 = 1,
    LEVEL_3 = 2,
    LEVEL_4 = 3,
    LEVEL_5 = 4,
    LEVEL_6 = 5,
    LEVEL_7 = 6,
    LEVEL_8 = 7,
    LEVEL_9 = 8,
    LEVEL_10 = 9,
    LEVEL_COUNT = 10
} LevelId;

typedef struct LevelObjective {
    char gate_id[LG_MAX_ID_LEN];
    int expected_state;
} LevelObjective;

typedef struct LevelInfo {
    LevelId id;
    const char* key;
    const char* label;
    size_t move_limit;
    LevelObjective objectives[LG_MAX_OBJECTIVES];
    size_t objective_count;
} LevelInfo;

typedef struct NodeSnapshot {
    char id[LG_MAX_ID_LEN];
    char type[16];
    int state;
    int toggleable;
    float x;
    float y;
} NodeSnapshot;

typedef struct EdgeSnapshot {
    char from_id[LG_MAX_ID_LEN];
    char to_id[LG_MAX_ID_LEN];
} EdgeSnapshot;

typedef enum ToggleStatus {
    TOGGLE_OK = 0,
    TOGGLE_MISSING = 1,
    TOGGLE_NOT_TOGGLEABLE = 2
} ToggleStatus;

typedef enum MoveStatus {
    MOVE_OK = 0,
    MOVE_MISSING = 1,
    MOVE_INVALID_COORDS = 2
} MoveStatus;

typedef struct Circuit {
    Gate gates[LG_MAX_GATES];
    size_t gate_count;
    size_t eval_order[LG_MAX_GATES];
    size_t eval_count;
    LevelId current_level;
    size_t move_count;
    size_t move_limit;
    int unlocked[LEVEL_COUNT];
    int won;
} Circuit;

void Circuit_Init(Circuit* circuit);
void Circuit_SeedDemo(Circuit* circuit);
void Circuit_LoadLevel(Circuit* circuit, LevelId level);
ToggleStatus Circuit_ToggleSwitch(Circuit* circuit, const char* gate_id);
MoveStatus Circuit_MoveGate(Circuit* circuit, const char* gate_id, float x, float y);
size_t Circuit_Snapshot(const Circuit* circuit, NodeSnapshot* out_nodes, size_t capacity);
size_t Circuit_SnapshotEdges(const Circuit* circuit, EdgeSnapshot* out_edges, size_t capacity);
int Circuit_IsWon(const Circuit* circuit);
int Circuit_IsLevelUnlocked(const Circuit* circuit, LevelId level);
const LevelInfo* Circuit_GetLevelInfo(LevelId level);
const LevelInfo* Circuit_GetCurrentLevelInfo(const Circuit* circuit);
int Circuit_LevelFromString(const char* text, LevelId* out_level);

#ifdef __cplusplus
}
#endif
