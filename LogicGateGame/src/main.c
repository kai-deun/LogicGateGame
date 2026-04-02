#include "engine/circuit.h"
#include "server/http_server.h"

int main(void) {
    Circuit circuit;

    const char* host = "localhost";
    const int port = 8080;

    Circuit_Init(&circuit);
    return HttpServer_Start(&circuit, host, port);
}
