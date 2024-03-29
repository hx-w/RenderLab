/**
 * mesh method
 */

#include <vector>
#include <string>

#include "geom_types.h"

namespace geometry {
    class Mesh: public GeometryBase {
    public:
        Mesh() = default;
        Mesh(const std::vector<Point3f>& vertices, const std::vector<Vector3u>& faces)
            : m_vertices(vertices), m_faces(faces) {}
        Mesh(const Mesh& mesh): m_vertices(mesh.m_vertices), m_faces(mesh.m_faces) {}

        static Mesh load_obj(const std::string& filename, int& status);

        static void save_obj(const std::string& filename, Mesh&, int& status);

        std::vector<Point3f> get_vertices() const { return m_vertices; }
        std::vector<Vector3u> get_faces() const { return m_faces; }
        std::vector<Point3f>& vertices() { return m_vertices; }
        std::vector<Vector3u>& faces() { return m_faces; }

        uint32_t hash() const;

    private:
        static void _split_words(const std::string&, std::vector<std::string>&, char);

    private:    
        std::vector<Point3f> m_vertices;
        std::vector<Vector3u> m_faces;
    };
}