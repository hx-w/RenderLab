#ifndef TOOTH_EXECUTE_H
#define TOOTH_EXECUTE_H
#include <string>

FILE* execute_base(const char* cmd);
std::string execute_short(const std::string& cmd);
void execute(const std::string& cmd);

#endif
