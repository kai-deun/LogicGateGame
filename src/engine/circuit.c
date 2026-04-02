#include "circuit.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const LevelInfo LG_LEVELS[LEVEL_COUNT] = {
    {
        LEVEL_EASY,
        "easy",
        "Easy",
        {
            {"Bulb1", 1}
        },
        1
    },
    {
        LEVEL_MEDIUM,
        "medium",
        "Medium",
        {
            {"And1", 1},
            {"Bulb1", 1}
        },
        2
    },
    {
        LEVEL_HARD,
        "hard",
        "Hard",
        {
            {"NotA", 0},
            {"And1", 0},
            {"Bulb1", 0}
        },
        3
    }
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

static void Circuit_BuildBaseNetwork(Circuit* circuit) {
    Gate* switch_a;
    Gate* switch_b;
    Gate* and_1;
    Gate* not_a;
    Gate* or_1;
    Gate* bulb_1;

    switch_a = Circuit_AddGate(circuit, "SwitchA", GATE_INPUT_SWITCH);
    switch_b = Circuit_AddGate(circuit, "SwitchB", GATE_INPUT_SWITCH);
    and_1 = Circuit_AddGate(circuit, "And1", GATE_AND);
    not_a = Circuit_AddGate(circuit, "NotA", GATE_NOT);
    or_1 = Circuit_AddGate(circuit, "Or1", GATE_OR);
    bulb_1 = Circuit_AddGate(circuit, "Bulb1", GATE_BULB);

    Gate_AddInput(and_1, switch_a);
    Gate_AddInput(and_1, switch_b);

    Gate_AddInput(not_a, switch_a);

    Gate_AddInput(or_1, and_1);
    Gate_AddInput(or_1, not_a);

    Gate_AddInput(bulb_1, or_1);
}

static void Circuit_ApplyLevelSeed(Circuit* circuit, LevelId level) {
    Gate* switch_a;
    Gate* switch_b;

    switch_a = Circuit_FindGate(circuit, "SwitchA");
    switch_b = Circuit_FindGate(circuit, "SwitchB");
    if (switch_a == NULL || switch_b == NULL) {
        return;
    }

    switch_a->state = 0;
    switch_b->state = 0;

    if (level == LEVEL_EASY) {
        switch_a->state = 1;
        switch_b->state = 0;
    } else if (level == LEVEL_MEDIUM) {
        switch_a->state = 1;
        switch_b->state = 0;
    } else {
        switch_a->state = 0;
        switch_b->state = 0;
    }
}

void Circuit_Init(Circuit* circuit) {
    if (circuit == NULL) {
        return;
    }

    memset(circuit, 0, sizeof(*circuit));
    circuit->unlocked[LEVEL_EASY] = 1;
    Circuit_LoadLevel(circuit, LEVEL_EASY);
}

void Circuit_SeedDemo(Circuit* circuit) {
    Circuit_LoadLevel(circuit, LEVEL_EASY);
}

void Circuit_LoadLevel(Circuit* circuit, LevelId level) {
    int unlocked_easy;
    int unlocked_medium;
    int unlocked_hard;

    if (circuit == NULL || level < LEVEL_EASY || level >= LEVEL_COUNT) {
        return;
    }

    unlocked_easy = circuit->unlocked[LEVEL_EASY];
    unlocked_medium = circuit->unlocked[LEVEL_MEDIUM];
    unlocked_hard = circuit->unlocked[LEVEL_HARD];

    memset(circuit, 0, sizeof(*circuit));
    circuit->unlocked[LEVEL_EASY] = unlocked_easy ? 1 : 1;
    circuit->unlocked[LEVEL_MEDIUM] = unlocked_medium;
    circuit->unlocked[LEVEL_HARD] = unlocked_hard;

    Circuit_BuildBaseNetwork(circuit);
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
    Circuit_Recompute(circuit);
    Circuit_RefreshWinAndUnlock(circuit);
    return TOGGLE_OK;
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
    }

    return count;
}

int Circuit_IsWon(const Circuit* circuit) {
    if (circuit == NULL) {
        return 0;
    }
    return circuit->won;
}

int Circuit_IsLevelUnlocked(const Circuit* circuit, LevelId level) {
    if (circuit == NULL || level < LEVEL_EASY || level >= LEVEL_COUNT) {
        return 0;
    }
    return circuit->unlocked[level] ? 1 : 0;
}

const LevelInfo* Circuit_GetLevelInfo(LevelId level) {
    if (level < LEVEL_EASY || level >= LEVEL_COUNT) {
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
