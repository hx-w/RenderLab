#ifndef PRINTER_H
#define PRINTER_H

#include <cassert>
#include <ostream>
#include <fstream>

namespace ToothSpace {
    class Printer {
    public:
        Printer() = default;
        Printer(const Printer&) = delete;
        Printer& operator=(const Printer&) = delete;
        Printer(const char* filename) {
            m_ofs.open(filename);
            assert(m_ofs.is_open());
        };
        ~Printer() {
            if (m_ofs.is_open()) {
                m_ofs.close();
            }
        }

        void to_csv(const char* sep=" ", ...);

    private:
        void _stream(std::ostream& ofs, const char* sep, ...);

    private:
        std::ofstream m_ofs;
    };
}

#endif
