#include "execute.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>

int execute(const LPTSTR childexe) {
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
        NULL, childexe, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi
    )) {
        CloseHandle(hWrite);
        CloseHandle(hRead);
        return false;
    }

    CloseHandle(hWrite);
    char ReadBuff[256] = {0};
    DWORD ReadNum = 0;
    while (ReadFile(hRead, ReadBuff, 256, &ReadNum, NULL)) {
        ReadBuff[ReadNum] = '\0';
        std::cout << ReadBuff;
    }
    return 0;

    // WaitForSingleObject(pi.hProcess,INFINITE);
    // CloseHandle(pi.hProcess);
    // CloseHandle(pi.hThread);
    // CloseHandle(hWrite);
    // ReadFile(hRead, buffer, MAX_PATH_LENGTH, &bytesRead, NULL);
    // CloseHandle(hRead);
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
