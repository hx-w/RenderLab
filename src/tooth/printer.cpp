#include "printer.h"
#include <cstdarg>

#include <iostream>

using namespace std;

namespace ToothSpace {
    void Printer::_stream(std::ostream& ofs, const char* sep, ...) {
        va_list args;
        va_start(args, sep);
        bool head = true;
        while (true) {
            cout << "begin" << endl;
            const char* str = va_arg(args, const char*);
            if (str == nullptr) {
                ofs << endl;
                break;
            }
            cout << "str: " << str << " ?" << endl;
            head ? head = false : (ofs << sep, true);
            ofs << str;
            cout << "end" << endl;
        }
        cout << "exit" << endl;
        va_end(args);
    }

    void Printer::to_csv(const char* sep, ...) {
        if (m_ofs.good()) {
            va_list args;
            va_start(args, sep);
            _stream(m_ofs, sep, args);
            va_end(args);
        }
    }
}
