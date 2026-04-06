#include "circuit.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct LevelSeed {
    int switch_a;
    int switch_b;
} LevelSeed;

static const LevelInfo LG_LEVELS[LEVEL_COUNT] = {
    {
        LEVEL_1,
        "level1",
        "Level 1: Split AND",
        0,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_2,
        "level2",
        "Level 2: AND to OR",
        1,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_3,
        "level3",
        "Level 3: OR to AND",
        0,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_4,
        "level4",
        "Level 4: NOT OR Split",
        2,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_5,
        "level5",
        "Level 5: Dual NOT",
        1,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_6,
        "level6",
        "Level 6: NAND Branch",
        0,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_7,
        "level7",
        "Level 7: AND NOR Deep",
        0,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_8,
        "level8",
        "Level 8: OR NOR Deep",
        1,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_9,
        "level9",
        "Level 9: NOR Entry (Bulb ON)",
        2,
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_10,
        "level10",
        "Level 10: Master Branch",
        3,
        {
            {"Bulb1", 1}
        },
        1
    }
};

static const LevelSeed LG_LEVEL_SEEDS[LEVEL_COUNT] = {
    {0, 1},
    {0, 0},
    {0, 1},
    {1, 0},
    {0, 0},
    {0, 1},
    {1, 1},
    {0, 0},
    {1, 0},
    {0, 1}
};

static Gate* Circuit_AddGate(Circuit* circuit, const char* id, GateType type) {
    Gate* gate;

    if (circuit == NULL || circuit->gate_count >= LG_MAX_GATES) {
        return NULL;
    }

    gate = &circuit->gates[circuit->gate_count];
    Gate_Init(gate, id, type);

    circuit->eval_order[circuit->eval_count++] = circuit->gate_count;
    circuit->gate_count++;
    return gate;
}

static Gate* Circuit_FindGate(Circuit* circuit, const char* id) {
    size_t i;

    if (circuit == NULL || id == NULL) {
        return NULL;
    }

    for (i = 0; i < circuit->gate_count; ++i) {
        if (strcmp(circuit->gates[i].id, id) == 0) {
            return &circuit->gates[i];
        }
    }

    return NULL;
}

static const Gate* Circuit_FindGateConst(const Circuit* circuit, const char* id) {
    size_t i;

    if (circuit == NULL || id == NULL) {
        return NULL;
    }

    for (i = 0; i < circuit->gate_count; ++i) {
        if (strcmp(circuit->gates[i].id, id) == 0) {
            return &circuit->gates[i];
        }
    }

    return NULL;
}

static float Circuit_ClampCoord(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static void Circuit_SetGatePosition(Circuit* circuit, const char* id, float x, float y) {
    Gate* gate = Circuit_FindGate(circuit, id);
    if (gate == NULL) {
        return;
    }
    gate->x = Circuit_ClampCoord(x);
    gate->y = Circuit_ClampCoord(y);
}

static void Circuit_ApplyDefaultLayout(Circuit* circuit, LevelId level) {
    if (circuit == NULL) {
        return;
    }

    Circuit_SetGatePosition(circuit, "SwitchA", 0.03f, 0.20f);
    Circuit_SetGatePosition(circuit, "SwitchB", 0.03f, 0.65f);
    Circuit_SetGatePosition(circuit, "Bulb1", 0.86f, 0.42f);

    switch (level) {
    case LEVEL_1:
        Circuit_SetGatePosition(circuit, "Gate1", 0.30f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.52f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.52f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.64f, 0.40f);
        break;
    case LEVEL_2:
        Circuit_SetGatePosition(circuit, "Gate1", 0.30f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.50f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.50f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.64f, 0.40f);
        break;
    case LEVEL_3:
        Circuit_SetGatePosition(circuit, "Gate1", 0.30f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.52f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.52f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.64f, 0.40f);
        break;
    case LEVEL_4:
        Circuit_SetGatePosition(circuit, "Gate1", 0.22f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.36f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.56f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.56f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.64f, 0.40f);
        break;
    case LEVEL_5:
        Circuit_SetGatePosition(circuit, "Gate1", 0.24f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.42f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.42f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.60f, 0.22f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.60f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate6", 0.68f, 0.40f);
        break;
    case LEVEL_6:
        Circuit_SetGatePosition(circuit, "Gate1", 0.18f, 0.28f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.34f, 0.34f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.54f, 0.14f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.56f, 0.58f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.70f, 0.36f);
        break;
    case LEVEL_7:
        Circuit_SetGatePosition(circuit, "Gate1", 0.22f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.42f, 0.24f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.42f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.60f, 0.24f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.66f, 0.40f);
        break;
    case LEVEL_8:
        Circuit_SetGatePosition(circuit, "Gate1", 0.20f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.38f, 0.24f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.56f, 0.24f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.56f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.66f, 0.40f);
        break;
    case LEVEL_9:
        Circuit_SetGatePosition(circuit, "Gate1", 0.18f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.34f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.50f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.62f, 0.24f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.62f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate6", 0.72f, 0.40f);
        break;
    case LEVEL_10:
        Circuit_SetGatePosition(circuit, "Gate1", 0.20f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate2", 0.34f, 0.20f);
        Circuit_SetGatePosition(circuit, "Gate3", 0.38f, 0.56f);
        Circuit_SetGatePosition(circuit, "Gate4", 0.50f, 0.08f);
        Circuit_SetGatePosition(circuit, "Gate5", 0.54f, 0.40f);
        Circuit_SetGatePosition(circuit, "Gate6", 0.64f, 0.60f);
        Circuit_SetGatePosition(circuit, "Gate7", 0.68f, 0.20f);
        break;
    default:
        Circuit_SetGatePosition(circuit, "Gate1", 0.35f, 0.40f);
        break;
    }
}

static void Circuit_Recompute(Circuit* circuit) {
    size_t i;

    if (circuit == NULL) {
        return;
    }

    for (i = 0; i < circuit->eval_count; ++i) {
        size_t idx = circuit->eval_order[i];
        if (idx < circuit->gate_count) {
            Gate_Evaluate(&circuit->gates[idx]);
        }
    }
}

static int Circuit_ComputeWinState(const Circuit* circuit) {
    const LevelInfo* info;
    size_t i;

    if (circuit == NULL) {
        return 0;
    }

    info = Circuit_GetCurrentLevelInfo(circuit);
    if (info == NULL) {
        return 0;
    }

    for (i = 0; i < info->objective_count; ++i) {
        const Gate* gate = Circuit_FindGateConst(circuit, info->objectives[i].gate_id);
        if (gate == NULL || gate->state != info->objectives[i].expected_state) {
            return 0;
        }
    }

    if (circuit->move_limit > 0 && circuit->move_count > circuit->move_limit) {
        return 0;
    }

    return 1;
}

static void Circuit_RefreshWinAndUnlock(Circuit* circuit) {
    if (circuit == NULL) {
        return;
    }

    circuit->won = Circuit_ComputeWinState(circuit);
    if (circuit->won && circuit->current_level + 1 < LEVEL_COUNT) {
        circuit->unlocked[circuit->current_level + 1] = 1;
    }
}

static void Circuit_BuildLevelNetwork(Circuit* circuit, LevelId level) {
    Gate* switch_a;
    Gate* switch_b;
    Gate* gate1;
    Gate* gate2;
    Gate* gate3;
    Gate* bulb;
    Gate* gate4;
    Gate* gate5;
    Gate* gate6;
    Gate* gate7;

    if (circuit == NULL) {
        return;
    }

    /* All levels have 2 input switches */
    switch_a = Circuit_AddGate(circuit, "SwitchA", GATE_INPUT_SWITCH);
    switch_b = Circuit_AddGate(circuit, "SwitchB", GATE_INPUT_SWITCH);

    /* Level-specific gate configurations */
    switch (level) {
        case LEVEL_1:
            /* Image 1 style: AND split and merge */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);

            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_AND);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_AND);
            Gate_AddInput(gate3, gate1);
            Gate_AddInput(gate3, switch_b);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_AND);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, gate3);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate4);
            break;

        case LEVEL_2:
            /* Image 2 style: AND to dual OR to OR */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);

            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_OR);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_OR);
            Gate_AddInput(gate3, gate1);
            Gate_AddInput(gate3, switch_b);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_OR);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, gate3);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate4);
            break;

        case LEVEL_3:
            /* Image 3 style: OR to dual AND to AND */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_OR);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);

            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_AND);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_AND);
            Gate_AddInput(gate3, gate1);
            Gate_AddInput(gate3, switch_b);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_AND);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, gate3);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate4);
            break;

        case LEVEL_4:
            /* Image 4 style: NOT + OR split to OR/AND */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_NOT);
            Gate_AddInput(gate1, switch_a);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_OR);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_b);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_OR);
            Gate_AddInput(gate3, gate2);
            Gate_AddInput(gate3, switch_a);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_AND);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, switch_b);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_AND);
            Gate_AddInput(gate5, gate3);
            Gate_AddInput(gate5, gate4);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate5);
            break;

        case LEVEL_5:
            /* Image 5 style: AND entry, dual NOT branches */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_NOT);
            Gate_AddInput(gate2, gate1);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_NOT);
            Gate_AddInput(gate3, switch_b);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_AND);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, switch_a);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_AND);
            Gate_AddInput(gate5, gate3);
            Gate_AddInput(gate5, switch_b);

            gate6 = Circuit_AddGate(circuit, "Gate6", GATE_OR);
            Gate_AddInput(gate6, gate4);
            Gate_AddInput(gate6, gate5);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate6);
            break;

        case LEVEL_6:
            /* Image 6 style: NOT->OR to OR/NAND */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_NOT);
            Gate_AddInput(gate1, switch_a);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_OR);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_b);

            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_OR);
            Gate_AddInput(gate3, gate2);
            Gate_AddInput(gate3, switch_a);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_NAND);
            Gate_AddInput(gate4, gate2);
            Gate_AddInput(gate4, switch_b);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_AND);
            Gate_AddInput(gate5, gate3);
            Gate_AddInput(gate5, gate4);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate5);
            break;

        case LEVEL_7:
            /* Image 7 style: deeper AND/NOR/NOT stack */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_AND);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);
            
            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_NOR);
            Gate_AddInput(gate3, gate1);
            Gate_AddInput(gate3, switch_b);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_NOT);
            Gate_AddInput(gate4, gate2);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_AND);
            Gate_AddInput(gate5, gate3);
            Gate_AddInput(gate5, gate4);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate5);
            break;

        case LEVEL_8:
            /* Image 8 style: OR centric with NOT and NOR branch */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_OR);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_OR);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);
            
            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_NOT);
            Gate_AddInput(gate3, gate2);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_AND);
            Gate_AddInput(gate4, gate1);
            Gate_AddInput(gate4, switch_b);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_NOR);
            Gate_AddInput(gate5, gate4);
            Gate_AddInput(gate5, gate3);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate5);
            break;

        case LEVEL_9:
            /* Image 9 style: NOR entry with AND/NOT/OR chain */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_NOR);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_AND);
            Gate_AddInput(gate2, switch_a);
            Gate_AddInput(gate2, switch_b);
            
            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_NOT);
            Gate_AddInput(gate3, gate2);

            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_OR);
            Gate_AddInput(gate4, gate3);
            Gate_AddInput(gate4, switch_b);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_OR);
            Gate_AddInput(gate5, gate2);
            Gate_AddInput(gate5, gate1);

            gate6 = Circuit_AddGate(circuit, "Gate6", GATE_AND);
            Gate_AddInput(gate6, gate4);
            Gate_AddInput(gate6, gate5);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate6);
            break;

        case LEVEL_10:
            /* Image 10 style: deep multi-branch master */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            gate2 = Circuit_AddGate(circuit, "Gate2", GATE_OR);
            Gate_AddInput(gate2, gate1);
            Gate_AddInput(gate2, switch_a);
            
            gate3 = Circuit_AddGate(circuit, "Gate3", GATE_AND);
            Gate_AddInput(gate3, gate1);
            Gate_AddInput(gate3, switch_b);
            
            gate4 = Circuit_AddGate(circuit, "Gate4", GATE_NOT);
            Gate_AddInput(gate4, gate2);

            gate5 = Circuit_AddGate(circuit, "Gate5", GATE_OR);
            Gate_AddInput(gate5, gate3);
            Gate_AddInput(gate5, gate4);

            gate6 = Circuit_AddGate(circuit, "Gate6", GATE_NAND);
            Gate_AddInput(gate6, gate5);
            Gate_AddInput(gate6, gate2);

            gate7 = Circuit_AddGate(circuit, "Gate7", GATE_AND);
            Gate_AddInput(gate7, gate6);
            Gate_AddInput(gate7, gate2);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate7);
            break;

        default:
            /* Fallback to simple AND gate */
            gate1 = Circuit_AddGate(circuit, "Gate1", GATE_AND);
            Gate_AddInput(gate1, switch_a);
            Gate_AddInput(gate1, switch_b);
            
            bulb = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);
            Gate_AddInput(bulb, gate1);
            break;
    }
}

static void Circuit_ApplyLevelSeed(Circuit* circuit, LevelId level) {
    Gate* switch_a;
    Gate* switch_b;
    LevelSeed seed;

    switch_a = Circuit_FindGate(circuit, "SwitchA");
    switch_b = Circuit_FindGate(circuit, "SwitchB");
    if (switch_a == NULL || switch_b == NULL) {
        return;
    }

    if (level < LEVEL_1 || level >= LEVEL_COUNT) {
        seed.switch_a = 0;
        seed.switch_b = 0;
    } else {
        seed = LG_LEVEL_SEEDS[level];
    }

    switch_a->state = seed.switch_a ? 1 : 0;
    switch_b->state = seed.switch_b ? 1 : 0;
}

void Circuit_Init(Circuit* circuit) {
    if (circuit == NULL) {
        return;
    }

    memset(circuit, 0, sizeof(*circuit));
    circuit->unlocked[LEVEL_1] = 1;
    Circuit_LoadLevel(circuit, LEVEL_1);
}

void Circuit_SeedDemo(Circuit* circuit) {
    Circuit_LoadLevel(circuit, LEVEL_1);
}

void Circuit_LoadLevel(Circuit* circuit, LevelId level) {
    int unlocked[LEVEL_COUNT];
    size_t i;
    const LevelInfo* level_info;

    if (circuit == NULL || level < LEVEL_1 || level >= LEVEL_COUNT) {
        return;
    }

    level_info = Circuit_GetLevelInfo(level);
    if (level_info == NULL) {
        return;
    }

    for (i = 0; i < LEVEL_COUNT; ++i) {
        unlocked[i] = circuit->unlocked[i] ? 1 : 0;
    }

    memset(circuit, 0, sizeof(*circuit));
    circuit->unlocked[LEVEL_1] = 1;
    for (i = 1; i < LEVEL_COUNT; ++i) {
        circuit->unlocked[i] = unlocked[i];
    }
    circuit->move_limit = level_info->move_limit;
    circuit->move_count = 0;

    Circuit_BuildLevelNetwork(circuit, level);
    Circuit_ApplyDefaultLayout(circuit, level);
    circuit->current_level = level;

    Circuit_ApplyLevelSeed(circuit, level);
    Circuit_Recompute(circuit);
    Circuit_RefreshWinAndUnlock(circuit);
}

ToggleStatus Circuit_ToggleSwitch(Circuit* circuit, const char* gate_id) {
    Gate* gate;

    if (circuit == NULL || gate_id == NULL || gate_id[0] == '\0') {
        return TOGGLE_MISSING;
    }

    gate = Circuit_FindGate(circuit, gate_id);
    if (gate == NULL) {
        return TOGGLE_MISSING;
    }
    if (!gate->toggleable) {
        return TOGGLE_NOT_TOGGLEABLE;
    }

    gate->state = !gate->state;
    circuit->move_count++;
    Circuit_Recompute(circuit);
    Circuit_RefreshWinAndUnlock(circuit);
    return TOGGLE_OK;
}

MoveStatus Circuit_MoveGate(Circuit* circuit, const char* gate_id, float x, float y) {
    Gate* gate;

    if (circuit == NULL || gate_id == NULL || gate_id[0] == '\0') {
        return MOVE_MISSING;
    }

    gate = Circuit_FindGate(circuit, gate_id);
    if (gate == NULL) {
        return MOVE_MISSING;
    }

    if (x < -4.0f || x > 4.0f || y < -4.0f || y > 4.0f) {
        return MOVE_INVALID_COORDS;
    }

    gate->x = Circuit_ClampCoord(x);
    gate->y = Circuit_ClampCoord(y);
    return MOVE_OK;
}

size_t Circuit_Snapshot(const Circuit* circuit, NodeSnapshot* out_nodes, size_t capacity) {
    size_t i;
    size_t count;

    if (circuit == NULL || out_nodes == NULL || capacity == 0) {
        return 0;
    }

    count = (circuit->eval_count < capacity) ? circuit->eval_count : capacity;
    for (i = 0; i < count; ++i) {
        size_t idx = circuit->eval_order[i];
        const Gate* gate;

        if (idx >= circuit->gate_count) {
            continue;
        }

        gate = &circuit->gates[idx];
        snprintf(out_nodes[i].id, sizeof(out_nodes[i].id), "%s", gate->id);
        snprintf(out_nodes[i].type, sizeof(out_nodes[i].type), "%s", GateType_ToString(gate->type));
        out_nodes[i].state = gate->state;
        out_nodes[i].toggleable = gate->toggleable;
        out_nodes[i].x = gate->x;
        out_nodes[i].y = gate->y;
    }

    return count;
}

size_t Circuit_SnapshotEdges(const Circuit* circuit, EdgeSnapshot* out_edges, size_t capacity) {
    size_t i;
    size_t written = 0;

    if (circuit == NULL || out_edges == NULL || capacity == 0) {
        return 0;
    }

    for (i = 0; i < circuit->gate_count; ++i) {
        size_t input_idx;
        const Gate* gate = &circuit->gates[i];

        for (input_idx = 0; input_idx < gate->input_count; ++input_idx) {
            const Gate* input = gate->inputs[input_idx];
            if (input == NULL || written >= capacity) {
                continue;
            }

            snprintf(out_edges[written].from_id, sizeof(out_edges[written].from_id), "%s", input->id);
            snprintf(out_edges[written].to_id, sizeof(out_edges[written].to_id), "%s", gate->id);
            written++;
        }
    }

    return written;
}

int Circuit_IsWon(const Circuit* circuit) {
    if (circuit == NULL) {
        return 0;
    }
    return circuit->won;
}

int Circuit_IsLevelUnlocked(const Circuit* circuit, LevelId level) {
    if (circuit == NULL || level < LEVEL_1 || level >= LEVEL_COUNT) {
        return 0;
    }
    return circuit->unlocked[level] ? 1 : 0;
}

const LevelInfo* Circuit_GetLevelInfo(LevelId level) {
    if (level < LEVEL_1 || level >= LEVEL_COUNT) {
        return NULL;
    }
    return &LG_LEVELS[level];
}

const LevelInfo* Circuit_GetCurrentLevelInfo(const Circuit* circuit) {
    if (circuit == NULL) {
        return NULL;
    }
    return Circuit_GetLevelInfo(circuit->current_level);
}

int Circuit_LevelFromString(const char* text, LevelId* out_level) {
    size_t i;
    char normalized[16];
    size_t n = 0;

    if (text == NULL || out_level == NULL) {
        return 0;
    }

    for (i = 0; text[i] != '\0' && n + 1 < sizeof(normalized); ++i) {
        normalized[n++] = (char) tolower((unsigned char) text[i]);
    }
    normalized[n] = '\0';

    for (i = 0; i < LEVEL_COUNT; ++i) {
        if (strcmp(normalized, LG_LEVELS[i].key) == 0) {
            *out_level = LG_LEVELS[i].id;
            return 1;
        }
    }

    return 0;
}
