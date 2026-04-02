#include "gate.h"

#include <stdio.h>
#include <string.h>

const char* GateType_ToString(GateType type) {
    switch (type) {
    case GATE_INPUT_SWITCH:
        return "InputSwitch";
    case GATE_AND:
        return "And";
    case GATE_OR:
        return "Or";
    case GATE_NOT:
        return "Not";
    case GATE_BULB:
        return "Bulb";
    default:
        return "Unknown";
    }
}

void Gate_Init(Gate* gate, const char* id, GateType type) {
    if (gate == NULL) {
        return;
    }

    memset(gate, 0, sizeof(*gate));
    snprintf(gate->id, sizeof(gate->id), "%s", id == NULL ? "" : id);
    gate->type = type;
    gate->toggleable = (type == GATE_INPUT_SWITCH) ? 1 : 0;
}

int Gate_AddInput(Gate* gate, Gate* input) {
    if (gate == NULL || gate->input_count >= LG_MAX_INPUTS) {
        return 0;
    }

    gate->inputs[gate->input_count++] = input;
    return 1;
}

static int FirstInputState(const Gate* gate, int fallback) {
    if (gate == NULL || gate->input_count == 0 || gate->inputs[0] == NULL) {
        return fallback;
    }
    return gate->inputs[0]->state;
}

int Gate_Evaluate(Gate* gate) {
    size_t i;
    int result;

    if (gate == NULL) {
        return 0;
    }

    switch (gate->type) {
    case GATE_INPUT_SWITCH:
        return gate->state;

    case GATE_AND:
        if (gate->input_count == 0) {
            gate->state = 0;
            return gate->state;
        }
        result = 1;
        for (i = 0; i < gate->input_count; ++i) {
            result = result && (gate->inputs[i] != NULL && gate->inputs[i]->state);
        }
        gate->state = result;
        return gate->state;

    case GATE_OR:
        result = 0;
        for (i = 0; i < gate->input_count; ++i) {
            result = result || (gate->inputs[i] != NULL && gate->inputs[i]->state);
        }
        gate->state = result;
        return gate->state;

    case GATE_NOT:
        gate->state = !FirstInputState(gate, 1);
        return gate->state;

    case GATE_BULB:
        gate->state = FirstInputState(gate, 0);
        return gate->state;

    default:
        gate->state = 0;
        return gate->state;
    }
}
