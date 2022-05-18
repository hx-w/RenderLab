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
        Printer(const string& filename, bool bin=false);
        ~Printer() {
            if (m_ofs.is_open()) {
                m_ofs.close();
            }
        }

        template <class ...Args>
        void to_csv(const Args&... args) {
            _stream(m_ofs, ",", true, args...);
        }

        template <class ...Args>
        static void to_console(const Args&... args) {
            _stream(std::cout, ", ", true, args...);
        }

        template <class T>
        void to_data(const T& data) {
            m_ofs.write(reinterpret_cast<const char*>(&data), sizeof(T));
        }

        template <class ...Args>
        static void info(const Args&... args) {
            _stream(std::cout, " ", true, "[INFO]", args...);
        }

        static void show_percient(const string& desc, double percent);

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
        std::ofstream m_ofs;
    };
}

#endif
