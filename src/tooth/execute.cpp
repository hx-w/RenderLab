#include "execute.h"
#include <iostream>

using namespace std;

#ifdef _WIN32

HANDLE execute_base(const char* cmd) {
    SECURITY_ATTRIBUTES sa = {0};
    HANDLE hRead = NULL, hWrite = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return false;
    }
    HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
    SetStdHandle(STD_OUTPUT_HANDLE, hTemp);

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    if (!CreateProcess(
        NULL, (char*)cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi
    )) {
        CloseHandle(hWrite);
        CloseHandle(hRead);
        return NULL;
    }
    CloseHandle(hWrite);
    return hRead;
}

int execute(const string& cmd) {
    HANDLE hProcess = execute_base(cmd.c_str());
    if (hProcess == NULL)
        return -1;
    WaitForSingleObject(hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);
    return exitCode;
}

string execute_short(const string& cmd) {
    HANDLE hProcess = execute_base(cmd.c_str());
    if (hProcess == NULL)
        return "";
    string result;
    char buf[1024];
    DWORD read;
    while (ReadFile(hProcess, buf, sizeof(buf), &read, NULL)) {
        if (strlen(buf) != 0) {
            buf[read] = '\0';
            result = string(buf);
            break;
        }
    }
    // check handle valid
    CloseHandle(hProcess);
    return result;
}

#else

FILE* execute_base(const char* cmd) {
    char ps[1024] = {0};
    FILE* ptr;
    strcpy(ps, cmd);
    ptr = popen(ps, "r");
    if (ptr == NULL) {
        printf("popen error\n");
    }
    return ptr;
}

void execute(const std::string& cmd) {
    FILE* ptr = execute_base(cmd.c_str());
    char buffer[1024] = {0};
    while (fgets(buffer, 1024, ptr) != NULL) {
        printf("%s", buffer);
    }
    if (ptr != NULL) {
        pclose(ptr);
    }
}

std::string execute_short(const std::string& cmd) {
    FILE* ptr = execute_base(cmd.c_str());
    char buffer[1024] = {0};
    while (fgets(buffer, 1024, ptr) != NULL) {
        if (strlen(buffer) != 0 && isdigit(buffer[0])) {
            break; // one line output
        }
    }
    if (ptr != NULL) {
        pclose(ptr);
    }
    return std::string(buffer);
}

#endif
