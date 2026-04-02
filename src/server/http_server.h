#pragma once

#include "../engine/circuit.h"

#ifdef __cplusplus
extern "C" {
#endif

int HttpServer_Start(Circuit* circuit, const char* host, int port);

#ifdef __cplusplus
}
#endif
