#include <iostream>
#include <fstream>
#include <functional>
#include "mesh.h"

namespace geometry {

    Mesh Mesh::load_obj(const std::string& filename, bool& status) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: cannot open file " << filename << std::endl;
            status = false;
            return Mesh();
        }
        std::string line;

        std::vector<Point3f> vertices;  
        std::vector<glm::uvec3> faces;
    
        while (std::getline(file, line)) {
            if (line[0] == 'v') {
                float x, y, z;
                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                vertices.emplace_back(Point3f(x, y, z));
            } else if (line[0] == 'f') {
                uint32_t a, b, c;
                sscanf(line.c_str(), "f %d %d %d", &a, &b, &c);
                faces.emplace_back(glm::uvec3(a - 1, b - 1, c - 1));
            }
        }
        status = true;
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