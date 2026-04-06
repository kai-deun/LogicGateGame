#include "engine/circuit.h"
#include "server/http_server.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32

#define LG_PATH_CAPACITY 1024

static int PathDirname(const char* path, char* out, size_t out_size) {
    size_t len;
    const char* slash;

    if (path == NULL || out == NULL || out_size == 0) {
        return 0;
    }

    len = strlen(path);
    slash = strrchr(path, '\\');
    if (slash == NULL) {
        slash = strrchr(path, '/');
    }
    if (slash == NULL) {
        return 0;
    }

    len = (size_t) (slash - path);
    if (len + 1 > out_size) {
        return 0;
    }

    memcpy(out, path, len);
    out[len] = '\0';
    return 1;
}

static int JoinPath(char* out, size_t out_size, const char* a, const char* b) {
    int written;
    if (out == NULL || out_size == 0 || a == NULL || b == NULL) {
        return 0;
    }
    written = snprintf(out, out_size, "%s\\%s", a, b);
    return written > 0 && (size_t) written < out_size;
}

static int FileTimeValue(const char* path, ULONGLONG* out_value) {
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    ULARGE_INTEGER uli;

    if (path == NULL || out_value == NULL) {
        return 0;
    }

    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attrs)) {
        return 0;
    }

    uli.HighPart = attrs.ftLastWriteTime.dwHighDateTime;
    uli.LowPart = attrs.ftLastWriteTime.dwLowDateTime;
    *out_value = uli.QuadPart;
    return 1;
}

static int SourcesNewerThanExe(const char* root_dir, const char* exe_path) {
    static const char* tracked_files[] = {
        "src\\main.c",
        "src\\engine\\circuit.c",
        "src\\engine\\circuit.h",
        "src\\engine\\gate.c",
        "src\\engine\\gate.h",
        "src\\server\\http_server.c",
        "src\\server\\http_server.h",
        "src\\server\\json_state.c",
        "src\\server\\json_state.h",
        "src\\server\\embedded_assets.c",
        "src\\server\\embedded_assets.h",
        "third_party\\mongoose.c",
        "third_party\\mongoose.h"
    };
    ULONGLONG exe_time;
    size_t i;

    if (!FileTimeValue(exe_path, &exe_time)) {
        return 0;
    }

    for (i = 0; i < sizeof(tracked_files) / sizeof(tracked_files[0]); ++i) {
        char file_path[LG_PATH_CAPACITY];
        ULONGLONG file_time;
        if (!JoinPath(file_path, sizeof(file_path), root_dir, tracked_files[i])) {
            continue;
        }
        if (!FileTimeValue(file_path, &file_time)) {
            continue;
        }
        if (file_time > exe_time) {
            return 1;
        }
    }

    return 0;
}

static int RunCommand(const char* root_dir, const char* command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdline[4096];
    DWORD exit_code = 1;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    snprintf(cmdline, sizeof(cmdline), "cmd.exe /C \"cd /d \"\"%s\"\" && set PATH=C:\\msys64\\mingw64\\bin;%%PATH%% && %s\"", root_dir, command);

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return 0;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exit_code == 0;
}

static int TryBuildFreshExe(const char* root_dir) {
    static const char* build_commands[] = {
        "gcc -std=c11 -Wall -Wextra -pedantic -Isrc -Ithird_party src\\main.c src\\engine\\circuit.c src\\engine\\gate.c src\\server\\http_server.c src\\server\\json_state.c src\\server\\embedded_assets.c third_party\\mongoose.c -lws2_32 -o LogicGateGame.new.exe",
        "clang -std=c11 -Wall -Wextra -pedantic -Isrc -Ithird_party src\\main.c src\\engine\\circuit.c src\\engine\\gate.c src\\server\\http_server.c src\\server\\json_state.c src\\server\\embedded_assets.c third_party\\mongoose.c -lws2_32 -o LogicGateGame.new.exe",
        "cl /nologo /std:c11 /W3 /Isrc /Ithird_party src\\main.c src\\engine\\circuit.c src\\engine\\gate.c src\\server\\http_server.c src\\server\\json_state.c src\\server\\embedded_assets.c third_party\\mongoose.c /link ws2_32.lib /OUT:LogicGateGame.new.exe"
    };
    size_t i;

    for (i = 0; i < sizeof(build_commands) / sizeof(build_commands[0]); ++i) {
        if (RunCommand(root_dir, build_commands[i])) {
            return 1;
        }
    }

    return 0;
}

static int ScheduleSwapAndRestart(const char* exe_path, const char* new_exe_path) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdline[4096];

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    snprintf(
        cmdline,
        sizeof(cmdline),
        "cmd.exe /C \"ping 127.0.0.1 -n 2 >nul && move /Y \"\"%s\"\" \"\"%s\"\" >nul && start \"\" \"\"%s\"\"\"",
        new_exe_path,
        exe_path,
        exe_path
    );

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return 0;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}

static int BootstrapBuildIfNeeded(void) {
    char exe_path[LG_PATH_CAPACITY];
    char root_dir[LG_PATH_CAPACITY];
    char new_exe_path[LG_PATH_CAPACITY];
    char skip_flag[8];

    if (GetEnvironmentVariableA("LG_SKIP_BOOTSTRAP", skip_flag, sizeof(skip_flag)) > 0) {
        return 0;
    }

    if (!GetModuleFileNameA(NULL, exe_path, (DWORD) sizeof(exe_path))) {
        return 0;
    }

    if (!PathDirname(exe_path, root_dir, sizeof(root_dir))) {
        return 0;
    }

    if (!SourcesNewerThanExe(root_dir, exe_path)) {
        return 0;
    }

    if (!TryBuildFreshExe(root_dir)) {
        MessageBoxA(
            NULL,
            "LogicGateGame detected newer source files but could not auto-build a new executable.\n\n"
            "Install a supported C compiler (gcc, clang, or MSVC cl) and reopen LogicGateGame.exe.",
            "LogicGateGame Auto-Build Failed",
            MB_OK | MB_ICONWARNING
        );
        return 0;
    }

    if (!JoinPath(new_exe_path, sizeof(new_exe_path), root_dir, "LogicGateGame.new.exe")) {
        return 0;
    }

    if (!ScheduleSwapAndRestart(exe_path, new_exe_path)) {
        return 0;
    }

    return 1;
}

#endif

int main(void) {
    Circuit circuit;

    const char* host = "localhost";
    const int port = 8080;

#ifdef _WIN32
    if (BootstrapBuildIfNeeded()) {
        return 0;
    }
#endif

    Circuit_Init(&circuit);
    return HttpServer_Start(&circuit, host, port);
}
