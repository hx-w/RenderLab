#ifndef NURBS_EXECUTE_HPP
#define NURBS_EXECUTE_HPP

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

void execute(const char* cmd) {
    char buf_ps[1024];
    char ps[1024] = {0};
    FILE* ptr;
    strcpy(ps, cmd);
    if ((ptr = popen(ps, "r")) != NULL) {
        while (fgets(buf_ps, 1024, ptr) != NULL) {
            printf("%s\n", buf_ps);
            if (g_stop == 1)
                break;
        }
        pclose(ptr);
        ptr = NULL;
    } else {
        printf("popen %s error\n", ps);
    }
}

#endif

#endif
