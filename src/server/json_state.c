#include "json_state.h"

#include <stdio.h>
#include <string.h>

static size_t AppendText(char* buffer, size_t capacity, size_t offset, const char* text);

static size_t AppendBool(char* buffer, size_t capacity, size_t offset, int value) {
    return AppendText(buffer, capacity, offset, value ? "true" : "false");
}

static size_t AppendText(char* buffer, size_t capacity, size_t offset, const char* text) {
    size_t i;

    if (buffer == NULL || text == NULL || offset >= capacity) {
        return offset;
    }

    for (i = 0; text[i] != '\0' && offset + 1 < capacity; ++i) {
        buffer[offset++] = text[i];
    }
    buffer[offset] = '\0';
    return offset;
}

static size_t AppendEscaped(char* buffer, size_t capacity, size_t offset, const char* text) {
    size_t i;

    if (buffer == NULL || text == NULL || offset >= capacity) {
        return offset;
    }

    for (i = 0; text[i] != '\0' && offset + 1 < capacity; ++i) {
        char c = text[i];
        if ((c == '\\' || c == '"') && offset + 2 < capacity) {
            buffer[offset++] = '\\';
            buffer[offset++] = c;
        } else if (c == '\n' && offset + 2 < capacity) {
            buffer[offset++] = '\\';
            buffer[offset++] = 'n';
        } else if (c == '\r' && offset + 2 < capacity) {
            buffer[offset++] = '\\';
            buffer[offset++] = 'r';
        } else if (c == '\t' && offset + 2 < capacity) {
            buffer[offset++] = '\\';
            buffer[offset++] = 't';
        } else {
            buffer[offset++] = c;
        }
    }

    if (offset < capacity) {
        buffer[offset] = '\0';
    }
    return offset;
}

size_t Json_BuildState(
    const Circuit* circuit,
    const NodeSnapshot* nodes,
    size_t node_count,
    const EdgeSnapshot* edges,
    size_t edge_count,
    char* buffer,
    size_t buffer_size
) {
    size_t i;
    size_t offset = 0;
    const LevelInfo* current;

    if (buffer == NULL || buffer_size == 0 || circuit == NULL) {
        return 0;
    }

    current = Circuit_GetCurrentLevelInfo(circuit);

    buffer[0] = '\0';
    offset = AppendText(buffer, buffer_size, offset, "{\"nodes\":[");

    for (i = 0; i < node_count; ++i) {
        const NodeSnapshot* node = &nodes[i];

        if (i > 0) {
            offset = AppendText(buffer, buffer_size, offset, ",");
        }

        offset = AppendText(buffer, buffer_size, offset, "{\"id\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, node->id);
        offset = AppendText(buffer, buffer_size, offset, "\",\"type\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, node->type);
        offset = AppendText(buffer, buffer_size, offset, "\",\"state\":");
        offset = AppendText(buffer, buffer_size, offset, node->state ? "true" : "false");
        offset = AppendText(buffer, buffer_size, offset, ",\"toggleable\":");
        offset = AppendText(buffer, buffer_size, offset, node->toggleable ? "true" : "false");
        offset = AppendText(buffer, buffer_size, offset, ",\"x\":");
        offset += snprintf(buffer + offset, buffer_size - offset, "%.4f", (double) node->x);
        offset = AppendText(buffer, buffer_size, offset, ",\"y\":");
        offset += snprintf(buffer + offset, buffer_size - offset, "%.4f", (double) node->y);
        offset = AppendText(buffer, buffer_size, offset, "}");
    }

    offset = AppendText(buffer, buffer_size, offset, "],\"edges\":[");
    for (i = 0; i < edge_count; ++i) {
        if (i > 0) {
            offset = AppendText(buffer, buffer_size, offset, ",");
        }
        offset = AppendText(buffer, buffer_size, offset, "{\"from\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, edges[i].from_id);
        offset = AppendText(buffer, buffer_size, offset, "\",\"to\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, edges[i].to_id);
        offset = AppendText(buffer, buffer_size, offset, "\"}");
    }

    offset = AppendText(buffer, buffer_size, offset, "],\"level\":\"");
    offset = AppendEscaped(buffer, buffer_size, offset, current == NULL ? "level1" : current->key);
    offset = AppendText(buffer, buffer_size, offset, "\",\"levelLabel\":\"");
    offset = AppendEscaped(buffer, buffer_size, offset, current == NULL ? "Level 1" : current->label);
    offset = AppendText(buffer, buffer_size, offset, "\",\"won\":");
    offset = AppendBool(buffer, buffer_size, offset, Circuit_IsWon(circuit));
    offset = AppendText(buffer, buffer_size, offset, ",\"moves\":");
    offset += snprintf(buffer + offset, buffer_size - offset, "%lu", (unsigned long) circuit->move_count);
    offset = AppendText(buffer, buffer_size, offset, ",\"moveLimit\":");
    offset += snprintf(buffer + offset, buffer_size - offset, "%lu", (unsigned long) circuit->move_limit);

    offset = AppendText(buffer, buffer_size, offset, ",\"unlocked\":{");
    for (i = 0; i < LEVEL_COUNT; ++i) {
        const LevelInfo* info = Circuit_GetLevelInfo((LevelId) i);
        if (i > 0) {
            offset = AppendText(buffer, buffer_size, offset, ",");
        }
        offset = AppendText(buffer, buffer_size, offset, "\"");
        offset = AppendEscaped(buffer, buffer_size, offset, info == NULL ? "unknown" : info->key);
        offset = AppendText(buffer, buffer_size, offset, "\":");
        offset = AppendBool(buffer, buffer_size, offset, Circuit_IsLevelUnlocked(circuit, (LevelId) i));
    }
    offset = AppendText(buffer, buffer_size, offset, "}");

    offset = AppendText(buffer, buffer_size, offset, ",\"objectives\":[");
    if (current != NULL) {
        for (i = 0; i < current->objective_count; ++i) {
            if (i > 0) {
                offset = AppendText(buffer, buffer_size, offset, ",");
            }
            offset = AppendText(buffer, buffer_size, offset, "{\"id\":\"");
            offset = AppendEscaped(buffer, buffer_size, offset, current->objectives[i].gate_id);
            offset = AppendText(buffer, buffer_size, offset, "\",\"expected\":");
            offset = AppendBool(buffer, buffer_size, offset, current->objectives[i].expected_state);
            offset = AppendText(buffer, buffer_size, offset, "}");
        }
    }
    offset = AppendText(buffer, buffer_size, offset, "]}");
    return offset;
}

size_t Json_BuildLevels(const Circuit* circuit, char* buffer, size_t buffer_size) {
    size_t i;
    size_t offset = 0;

    if (circuit == NULL || buffer == NULL || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';
    offset = AppendText(buffer, buffer_size, offset, "{\"levels\":[");
    for (i = 0; i < LEVEL_COUNT; ++i) {
        const LevelInfo* info = Circuit_GetLevelInfo((LevelId) i);
        if (info == NULL) {
            continue;
        }
        if (i > 0) {
            offset = AppendText(buffer, buffer_size, offset, ",");
        }
        offset = AppendText(buffer, buffer_size, offset, "{\"id\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, info->key);
        offset = AppendText(buffer, buffer_size, offset, "\",\"label\":\"");
        offset = AppendEscaped(buffer, buffer_size, offset, info->label);
        offset = AppendText(buffer, buffer_size, offset, "\",\"moveLimit\":");
        offset += snprintf(buffer + offset, buffer_size - offset, "%lu", (unsigned long) info->move_limit);
        offset = AppendText(buffer, buffer_size, offset, ",\"unlocked\":");
        offset = AppendBool(buffer, buffer_size, offset, Circuit_IsLevelUnlocked(circuit, info->id));
        offset = AppendText(buffer, buffer_size, offset, "}");
    }
    offset = AppendText(buffer, buffer_size, offset, "]}");
    return offset;
}

size_t Json_BuildError(const char* message, char* buffer, size_t buffer_size) {
    size_t offset = 0;

    if (buffer == NULL || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';
    offset = AppendText(buffer, buffer_size, offset, "{\"error\":\"");
    offset = AppendEscaped(buffer, buffer_size, offset, message == NULL ? "Unknown error" : message);
    offset = AppendText(buffer, buffer_size, offset, "\"}");
    return offset;
}
