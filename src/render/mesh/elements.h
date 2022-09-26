#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include "drawable.h"

namespace RenderSpace {
    class MeshDrawable : public Drawable {
    public:
        MeshDrawable() = default;
        MeshDrawable(const std::string& name, DrawableType type):
            Drawable(name, type) {}
        ~MeshDrawable() = default;
        // 拷贝构造不允许拷贝vao
        MeshDrawable(const MeshDrawable&) = default;
        MeshDrawable& operator=(const MeshDrawable&) = default;

        bool load_STL(const std::string& filename);
        bool load_OBJ(const std::string& filename);
        bool save_OBJ(const std::string& filename);

        // 根据原始坐标点创建顶点以及顶点索引三角形
        void add_triangle_raw(const Vertex& v1, const Vertex& v2, const Vertex& v3);
        void add_vertex_raw(const Vertex& v);
        void add_edge_raw(const Vertex& v1, const Vertex& v2);

        void add_triangle_by_idx(const Triangle& tri);

        void remesh();

    private:
        void _deepcopy(const MeshDrawable&);
        bool _read_STL_ASCII(const std::string& filename);
        bool _read_STL_Binary(const std::string& filename);
        void _split_words(const std::string& line, std::vector<std::string>& words, const char delim=' ');

        bool _remesh_check() const;

    private:
        std::string m_filename;
    };
}

#endif
