#ifndef DRAWABLE_H
#define DRAWABLE_H
#include <mutex>
#include <vector>
#include <string>
#include <algorithm>
#include "../shader.hpp"
#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    typedef std::pair<glm::vec3, glm::vec3> AABB; // min, max
    typedef std::pair<int, int> OrderedEdge; // v1, v2; v1<v2

    enum DrawableType {
        DRAWABLE_POINT,
        DRAWABLE_LINE,
        DRAWABLE_TRIANGLE
    };

    struct Vertex {
        Vertex() = default;
        Vertex(
            const glm::vec3& pos,
            const glm::vec3& clr,
            const glm::vec3& nml) :
            Position(pos), Color(clr), Normal(nml) { }
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec3 Normal;
    };

    struct Edge {
        Edge() = default;
        Edge(int v0idx, int v1idx):
            VertexIdx(glm::uvec2(v0idx, v1idx)) { }
        bool operator==(const Edge& other) const {
            return (VertexIdx.x == other.VertexIdx.x && VertexIdx.y == other.VertexIdx.y) \
                || (VertexIdx.x == other.VertexIdx.y && VertexIdx.y == other.VertexIdx.x);
        }
        bool operator<(const Edge& other) const {
            return VertexIdx.x < other.VertexIdx.x || (VertexIdx.x == other.VertexIdx.x && VertexIdx.y < other.VertexIdx.y);
        }

        glm::uvec2 VertexIdx;
    };

    struct Triangle {
        Triangle() = default;
        // 顶点顺序不能变
        Triangle(int v0, int v1, int v2):
            VertexIdx(glm::uvec3(v0, v1, v2)) { }
        bool operator==(const Triangle& other) const {
            return (VertexIdx.x == other.VertexIdx.x && VertexIdx.y == other.VertexIdx.y && VertexIdx.z == other.VertexIdx.z) \
                || (VertexIdx.x == other.VertexIdx.x && VertexIdx.y == other.VertexIdx.z && VertexIdx.z == other.VertexIdx.y) \
                || (VertexIdx.x == other.VertexIdx.y && VertexIdx.y == other.VertexIdx.z && VertexIdx.z == other.VertexIdx.x) \
                || (VertexIdx.x == other.VertexIdx.y && VertexIdx.y == other.VertexIdx.x && VertexIdx.z == other.VertexIdx.z) \
                || (VertexIdx.x == other.VertexIdx.z && VertexIdx.y == other.VertexIdx.x && VertexIdx.z == other.VertexIdx.y) \
                || (VertexIdx.x == other.VertexIdx.z && VertexIdx.y == other.VertexIdx.y && VertexIdx.z == other.VertexIdx.x);
        }
        bool operator<(const Triangle& other) const {
            return VertexIdx.x < other.VertexIdx.x;
        }

        glm::uvec3 VertexIdx;
    };

    // base class for drawable objects
    class Drawable {
    public:
        Drawable();
        Drawable(const std::string& name, DrawableType type):
            m_name(name), m_type(type) { }
        ~Drawable();

        Drawable(const Drawable&);
        Drawable& operator=(const Drawable&);

        void set_type(DrawableType type);

        void set_shader(Shader& shader);
        std::string get_name() const {
            return m_name;
        };

        std::vector<Triangle>& get_triangles() {
            return m_triangles;
        }
        std::vector<Vertex>& get_vertices() {
            return m_vertices;
        }

        // hide or show
        void set_visible(bool visible);

        // 先ready update -> sync -> draw 
        void ready_to_update();

        // sync vertex/triangle data to vao vbo ebo
        void sync();

        void draw();

    protected:
        void _gen_vao();
        void _reset();
        void _deepcopy(const Drawable& element);

    protected:
        Shader m_shader;
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;

        std::vector<Triangle> m_triangles;
        std::vector<Edge> m_edges;
        std::vector<Vertex> m_vertices;
        glm::vec3 m_center;
        float m_radius;
        AABB m_aabb;

        std::string m_name;
        std::mutex m_mutex;

        bool _ready_to_update = false;
        bool _ready_to_draw = false;

        DrawableType m_type;
    };
}

#endif
