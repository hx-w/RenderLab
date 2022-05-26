#ifndef TOOTH_EXECUTE_H
#define TOOTH_EXECUTE_H
#include <string>

#ifdef _WIN32
#include <windows.h>
HANDLE execute_base(const char* cmd);
int execute(const std::string& cmd);
#else
FILE* execute_base(const char* cmd);
void execute(const std::string& cmd);
#endif

std::string execute_short(const std::string& cmd);

#endif
