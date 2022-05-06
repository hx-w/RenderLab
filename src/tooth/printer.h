#ifndef PRINTER_H
#define PRINTER_H

#include <cassert>
#include <ostream>
#include <fstream>
#include <string>

namespace ToothSpace {
    using std::string;
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

        template <class ...Args>
        void to_csv(const Args&... args) {
            _stream(m_ofs, ",", true, args...);
        }

    private:
        template <class T, class ...Args>
        void _stream(std::ostream& ofs, const string& sep, bool first, T head, Args... args) {
            if constexpr(sizeof...(args) > 0) {
                if (!first) {
                    ofs << sep;
                }
                ofs << head << sep;
                _stream(ofs, sep, false, args...);
            }
            else {
                ofs << head << std::endl;
            }
        }

    private:
        std::ofstream m_ofs;
    };
}

#endif
