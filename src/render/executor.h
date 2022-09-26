#ifndef RENDER_EXECUTOR
#define RENDER_EXECUTOR

#include <string>

namespace RenderSpace {

#ifdef _WIN32
#include <windows.h>

HANDLE execute_base(const char* cmd);
int execute(const std::string& cmd);
#else
FILE* execute_base(const char* cmd);
void execute(const std::string& cmd);
#endif

std::string execute_short(const std::string& cmd);

template <class ...Args>
decltype(auto)
command(const std::string& format, const Args&... args) {
    char buf[1024] = "";
    sprintf(buf, format.c_str(), args...);
    return execute(buf);
}

}

#endif