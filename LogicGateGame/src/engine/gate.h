#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LG_MAX_INPUTS 4
#define LG_MAX_ID_LEN 32

typedef enum GateType {
    GATE_INPUT_SWITCH = 0,
    GATE_AND = 1,
    GATE_OR = 2,
    GATE_NOT = 3,
    GATE_BULB = 4
} GateType;

typedef struct Gate {
    char id[LG_MAX_ID_LEN];
    GateType type;
    int state;
    int toggleable;
    struct Gate* inputs[LG_MAX_INPUTS];
    size_t input_count;
} Gate;

const char* GateType_ToString(GateType type);
void Gate_Init(Gate* gate, const char* id, GateType type);
int Gate_AddInput(Gate* gate, Gate* input);
int Gate_Evaluate(Gate* gate);

#ifdef __cplusplus
}
#endif
