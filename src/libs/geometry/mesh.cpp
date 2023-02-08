#include <iostream>
#include <fstream>
#include <functional>
#include <sstream>

#include "mesh.h"

using namespace std;

namespace geometry {

    Mesh Mesh::load_obj(const string& filename, int& status) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file " << filename << std::endl;
            status = false;
            return Mesh();
        }

        std::vector<Point3f> vertices;  
        std::vector<Vector3u> faces;

        string line;

        status = 0;
        try {
			while (getline(file, line)) {
				if (line.empty()) {
					continue;
				}
				if (line[0] == '#') {
					continue;
				}
				vector<string> words;
				_split_words(line, words, ' ');

				if (words[0] == "v") {
					vertices.emplace_back(Point3f(stof(words[1]), stof(words[2]), stof(words[3])));
				}
				else if (words[0] == "f") {
					faces.emplace_back(Vector3f(
						stof(words[1]) - 1, stof(words[2]) - 1, stof(words[3]) - 1
					));
				}
			}
            status = 2;
        }
        catch (exception& e) {
            clog << "load mesh err: " << e.what() << endl;
        }

        file.close();
        return Mesh(vertices, faces);
    }

    void Mesh::save_obj(const string& filename, Mesh& msh, int& status) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: cannot open file " << filename << std::endl;
            status = 0;
            return;
        }

        status = 0;

        try {
            const auto& verts = msh.vertices();
            const auto& faces = msh.faces();

            for (auto& v : verts) {
                file << "v " << v.x << " " << v.y << " " << v.z << endl;
            }
            for (auto& f : faces) {
                file << "f " << f.x + 1 << " " << f.y + 1 << " " << f.z + 1 << endl;
            }
            status = 1;
        }
        catch (exception& e) {
            clog << "save mesh err: " << e.what() << endl;
        }

        file.close();
    }

    uint32_t Mesh::hash() const {
        auto vs = m_vertices.size();
        uint32_t _h = 0xdeadbeef;
        for (auto i = 0; i < vs; ++i) {
            _h ^= std::hash<float>()(m_vertices[i].x);
        }
        return _h;
    }

    void Mesh::_split_words(const string& line, vector<string>& words, char delim) {
        stringstream ss(line);
        string word = "";
        while (getline(ss, word, delim)) {
            words.emplace_back(word);
        }
    }
}