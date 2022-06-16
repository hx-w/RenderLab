#ifndef DRAWABLE_H
#define DRAWABLE_H
#include <mutex>
#include <vector>
#include <string>
#include <atomic>
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

    enum ColorMode {
        CM_Original,
        CM_Static_Random,
        CM_Dynamic_Random,
        CM_ColorMap_Gauss,
        CM_ColorMap_Mean
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
        glm::vec3 BufColor; // for color map or random color
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

        int vt_in(int vt) const {
            if (VertexIdx.x == vt) return 0;
            if (VertexIdx.y == vt) return 1;
            if (VertexIdx.z == vt) return 2;
            return -1;
        }

        bool is_neighbor(const Triangle& tri) {
            int x_ret = vt_in(tri.VertexIdx.x);
            int y_ret = vt_in(tri.VertexIdx.y);
            int z_ret = vt_in(tri.VertexIdx.z);
            return ((x_ret >= 0 && y_ret >= 0) || (x_ret >= 0 && z_ret >= 0) || (y_ret >= 0 && z_ret >= 0));
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

        void set_shade_mode(GLenum mode);
        GLenum get_shade_mode() { return m_shade_mode; }
        void set_color_mode(ColorMode mode);
        ColorMode get_color_mode() { return m_color_mode; }
        void sample_curvs(std::vector<float>& values, float sample_rate) const;

        std::string get_name() const { return m_name; }

        AABB get_BBOX() const { return m_aabb; }

        std::vector<Triangle>& get_triangles() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_triangles;
        }
        std::vector<Vertex>& get_vertices() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_vertices;
        }

        // called when ready to update
        void compute_BBOX();

        // hide or show
        void set_visible(bool visible);
        bool is_visible() const;

        // 先ready update -> sync -> draw 
        void ready_to_update();

        // sync vertex/triangle data to vao vbo ebo
        void sync();

        void draw();

    protected:
        void _gen_vao();
        void _reset();
        void _deepcopy(const Drawable& element);

        // 可视化
        void buf_colormap(ColorMode);
        void compute_curvs(int mode);

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

        std::atomic<bool> _ready_to_update = false;
        std::atomic<bool> _ready_to_draw = false;

        DrawableType m_type;

        std::vector<float> m_curvs; // 曲率(归一化后)
        // properties
        GLenum m_shade_mode = GL_LINE;
        ColorMode m_color_mode = CM_Original;
    };
}

#endif
