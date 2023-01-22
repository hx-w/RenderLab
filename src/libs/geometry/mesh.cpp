#include <iostream>
#include <fstream>
#include <functional>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include "mesh.h"

namespace py = pybind11;

namespace geometry {

    Mesh Mesh::load_obj(const std::string& filename, int& status) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file " << filename << std::endl;
            status = false;
            return Mesh();
        }

        std::vector<Point3f> vertices;  
        std::vector<Vector3u> faces;

        status = 0;
        try {
            py::scoped_interpreter guard{};
            // load by py module [trimesh]
            auto py_trimesh = py::module_::import("trimesh");
            auto _mesh = py_trimesh.attr("load")(filename.c_str());

            /**
             * directly access pymem, must split into two steps
             * 1. cast raw data into py::array_t<T>
             * 2. make py::array_t<T> unchecked
             * if merge these two steps into one,
             * `_faces(row, col)` will get a wrong output
             * 
             * @date 2023.01.09
             */
            auto py_verts = _mesh.attr("vertices").cast<py::array_t<float>>();
            auto py_faces = _mesh.attr("faces").cast<py::array_t<uint32_t>>();

            auto _verts = py_verts.unchecked<2>();
            auto _faces = py_faces.unchecked<2>();

            for (auto _row = 0; _row < _verts.shape(0); ++_row) {
                vertices.emplace_back(Point3f(_verts(_row, 0), _verts(_row, 1), _verts(_row, 2)));
            }
            for (auto _row = 0; _row < _faces.shape(0); ++_row) {
                faces.emplace_back(Vector3u(_faces(_row, 0), _faces(_row, 1), _faces(_row, 2)));
            }
            status++;
        }
        catch (const std::exception& e) {
            vertices.clear();
            faces.clear();
            std::string line;
            while (std::getline(file, line)) {
                if (line[0] == 'v') {
                    float x, y, z;
                    sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                    vertices.emplace_back(Point3f(x, y, z));
                }
                else if (line[0] == 'f') {
                    uint32_t a, b, c;
                    // [BUG] todo
                    sscanf(line.c_str(), "f %d %d %d", &a, &b, &c);
                    faces.emplace_back(Vector3u(a - 1, b - 1, c - 1));
                }
            }
            status++;
        }
        return Mesh(vertices, faces);
    }

    uint32_t Mesh::hash() const {
        auto vs = m_vertices.size();
        uint32_t _h = 0xdeadbeef;
        for (auto i = 0; i < vs; ++i) {
            _h ^= std::hash<float>()(m_vertices[i].x);
        }
        return _h;
    }



}