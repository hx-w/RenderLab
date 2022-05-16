#include "printer.h"

using namespace std;

namespace ToothSpace {
    Printer::Printer(const string& filename, bool bin) {
        if (bin) {
            m_ofs.open(filename, std::ios::binary);
        } else {
            m_ofs.open(filename);
        }
        assert(m_ofs.is_open());
    }

    void Printer::show_percient(const string& desc, double percent) {
        printf("[%s] %.2f%%\n", desc.c_str(), percent * 100.0);
    }
}
