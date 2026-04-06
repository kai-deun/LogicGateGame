#pragma once

#include "../engine/circuit.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t Json_BuildState(
	const Circuit* circuit,
	const NodeSnapshot* nodes,
	size_t node_count,
	const EdgeSnapshot* edges,
	size_t edge_count,
	char* buffer,
	size_t buffer_size
);
size_t Json_BuildLevels(const Circuit* circuit, char* buffer, size_t buffer_size);
size_t Json_BuildError(const char* message, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif
