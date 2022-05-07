#include "printer.h"

using namespace std;

namespace ToothSpace {
    void Printer::save_buffer() {
        if (m_ofs.good()) {
            m_ofs.close();
            m_ofs.open(m_filename.c_str(), std::ios::app);
        }
    }
}
