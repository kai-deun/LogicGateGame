#include "http_server.h"

#include "embedded_assets.h"
#include "json_state.h"
#include "mongoose.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#define LG_MAX_JSON 8192
#define LG_MAX_QUERY 128

typedef struct ServerContext {
    Circuit* circuit;
} ServerContext;

static void OpenBrowserTab(const char* url) {
#ifdef _WIN32
    static int launched = 0;
    if (launched) {
        return;
    }
    launched = 1;
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#else
    (void) url;
#endif
}

static void ReplyError(struct mg_connection* c, int status, const char* message) {
    char payload[LG_MAX_JSON];
    Json_BuildError(message, payload, sizeof(payload));
    mg_http_reply(c, status, "Content-Type: application/json\r\n", "%s", payload);
}

static void HandleState(struct mg_connection* c, ServerContext* context) {
    NodeSnapshot nodes[LG_MAX_GATES];
    EdgeSnapshot edges[LG_MAX_EDGES];
    size_t count;
    size_t edge_count;
    char payload[LG_MAX_JSON];

    count = Circuit_Snapshot(context->circuit, nodes, LG_MAX_GATES);
    edge_count = Circuit_SnapshotEdges(context->circuit, edges, LG_MAX_EDGES);
    Json_BuildState(context->circuit, nodes, count, edges, edge_count, payload, sizeof(payload));
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", payload);
}

static void HandleLevels(struct mg_connection* c, ServerContext* context) {
    char payload[LG_MAX_JSON];
    Json_BuildLevels(context->circuit, payload, sizeof(payload));
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", payload);
}

static void HandleSelectLevel(struct mg_connection* c, struct mg_http_message* hm, ServerContext* context) {
    char level_text[LG_MAX_QUERY];
    LevelId level;
    int len;

    len = mg_http_get_var(&hm->query, "level", level_text, sizeof(level_text));
    if (len <= 0) {
        ReplyError(c, 400, "Missing required query parameter: level");
        return;
    }

    if (!Circuit_LevelFromString(level_text, &level)) {
        ReplyError(c, 400, "Invalid level value. Use level1 through level10");
        return;
    }

    if (!Circuit_IsLevelUnlocked(context->circuit, level)) {
        ReplyError(c, 403, "Level is locked. Complete previous levels first");
        return;
    }

    Circuit_LoadLevel(context->circuit, level);
    HandleState(c, context);
}

static void HandleToggle(struct mg_connection* c, struct mg_http_message* hm, ServerContext* context) {
    char gate_id[LG_MAX_QUERY];
    int len;
    ToggleStatus status;

    len = mg_http_get_var(&hm->query, "name", gate_id, sizeof(gate_id));
    if (len <= 0) {
        ReplyError(c, 400, "Missing required query parameter: name");
        return;
    }

    status = Circuit_ToggleSwitch(context->circuit, gate_id);
    if (status == TOGGLE_MISSING) {
        ReplyError(c, 404, "Unknown gate id");
        return;
    }
    if (status == TOGGLE_NOT_TOGGLEABLE) {
        ReplyError(c, 409, "Gate is not toggleable");
        return;
    }

    HandleState(c, context);
}

static void HandleMove(struct mg_connection* c, struct mg_http_message* hm, ServerContext* context) {
    char gate_id[LG_MAX_QUERY];
    char x_text[LG_MAX_QUERY];
    char y_text[LG_MAX_QUERY];
    char* end = NULL;
    float x;
    float y;
    MoveStatus status;

    if (mg_http_get_var(&hm->query, "id", gate_id, sizeof(gate_id)) <= 0) {
        ReplyError(c, 400, "Missing required query parameter: id");
        return;
    }

    if (mg_http_get_var(&hm->query, "x", x_text, sizeof(x_text)) <= 0 ||
        mg_http_get_var(&hm->query, "y", y_text, sizeof(y_text)) <= 0) {
        ReplyError(c, 400, "Missing required query parameters: x and y");
        return;
    }

    x = (float) strtod(x_text, &end);
    if (end == NULL || *end != '\0') {
        ReplyError(c, 400, "Invalid x coordinate");
        return;
    }

    y = (float) strtod(y_text, &end);
    if (end == NULL || *end != '\0') {
        ReplyError(c, 400, "Invalid y coordinate");
        return;
    }

    status = Circuit_MoveGate(context->circuit, gate_id, x, y);
    if (status == MOVE_MISSING) {
        ReplyError(c, 404, "Unknown gate id");
        return;
    }
    if (status == MOVE_INVALID_COORDS) {
        ReplyError(c, 400, "Invalid coordinate range");
        return;
    }

    HandleState(c, context);
}

static void EventHandler(struct mg_connection* c, int ev, void* ev_data) {
    ServerContext* context = (ServerContext*) c->fn_data;

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*) ev_data;

        if (mg_strcmp(hm->uri, mg_str("/state")) == 0) {
            HandleState(c, context);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/levels")) == 0) {
            HandleLevels(c, context);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/toggle")) == 0) {
            HandleToggle(c, hm, context);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/select-level")) == 0) {
            HandleSelectLevel(c, hm, context);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/move")) == 0) {
            HandleMove(c, hm, context);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/")) == 0 || mg_strcmp(hm->uri, mg_str("/index.html")) == 0) {
            mg_http_reply(c, 200, "Content-Type: text/html; charset=utf-8\r\n", "%.*s", (int) LG_INDEX_HTML_LEN, LG_INDEX_HTML);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/styles.css")) == 0) {
            mg_http_reply(c, 200, "Content-Type: text/css; charset=utf-8\r\n", "%.*s", (int) LG_STYLES_CSS_LEN, LG_STYLES_CSS);
            return;
        }

        if (mg_strcmp(hm->uri, mg_str("/app.js")) == 0) {
            mg_http_reply(c, 200, "Content-Type: text/javascript; charset=utf-8\r\n", "%.*s", (int) LG_APP_JS_LEN, LG_APP_JS);
            return;
        }

        mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{\"error\":\"Not found\"}");
    }
}

int HttpServer_Start(Circuit* circuit, const char* host, int port) {
    struct mg_mgr mgr;
    struct mg_connection* conn;
    ServerContext context;
    char address[64];

    if (circuit == NULL || host == NULL) {
        return 1;
    }

    snprintf(address, sizeof(address), "http://%s:%d", host, port);

    context.circuit = circuit;

    mg_mgr_init(&mgr);
    conn = mg_http_listen(&mgr, address, EventHandler, &context);
    if (conn == NULL) {
        fprintf(stderr, "Failed to start server at %s\n", address);
        mg_mgr_free(&mgr);
        return 1;
    }

    printf("LogicGateGame server running at %s\n", address);
    printf("Serving embedded GUI assets from memory\n");

    OpenBrowserTab(address);

    for (;;) {
        mg_mgr_poll(&mgr, 100);
    }

    mg_mgr_free(&mgr);
    return 0;
}
