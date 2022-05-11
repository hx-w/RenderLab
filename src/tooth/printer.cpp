#include "printer.h"

using namespace std;

namespace ToothSpace {
    void Printer::save_buffer() {
        if (m_ofs.good()) {
            m_ofs.close();
            m_ofs.open(m_filename.c_str(), std::ios::app);
        }
    }

    void Printer::show_percient(const string& desc, double percent) {
        printf("[%s] %.2f%%\n", desc.c_str(), percent * 100.0);
    }
}
