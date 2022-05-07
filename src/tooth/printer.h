#ifndef PRINTER_H
#define PRINTER_H

#include <cassert>
#include <string>
#include <ostream>
#include <fstream>
#include <iostream>

namespace ToothSpace {
    using std::string;
    class Printer {
    public:
        Printer() = default;
        Printer(const Printer&) = delete;
        Printer& operator=(const Printer&) = delete;
        Printer(const string& filename): m_filename(filename) {
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

        void save_buffer();

        template <class ...Args>
        static void to_console(const Args&... args) {
            _stream(std::cout, ", ", true, args...);
        }

    private:
        template <class T, class ...Args>
        static void _stream(std::ostream& ofs, const string& sep, bool first, T head, Args... args) {
            if (!first) {
                ofs << sep;
            }
            if constexpr(sizeof...(args) > 0) {
                ofs << head;
                _stream(ofs, sep, false, args...);
            }
            else {
                ofs << head << std::endl;
            }
        }

    private:
        std::string m_filename;
        std::ofstream m_ofs;
    };
}

#endif
