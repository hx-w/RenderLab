#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include "drawable.h"

namespace RenderSpace {
    class MeshDrawable : public Drawable {
    public:
        MeshDrawable();
        ~MeshDrawable();
        MeshDrawable(const MeshDrawable&);
        MeshDrawable& operator=(const MeshDrawable&);

        void draw() override;

        bool load_STL(const std::string& filename);

        void ready_to_update();

        // sync vertex/triangle data to vao vbo ebo
        void sync();
        
        // 根据原始坐标点创建顶点以及顶点索引三角形
        void add_triangle_raw(const Vertex& v1, const Vertex& v2, const Vertex& v3);

        void add_triangle_by_idx(const Triangle& tri);
        void add_vertex_raw(const Vertex& v);
    private:
        bool _read_STL_ASCII(const std::string& filename);
        bool _read_STL_Binary(const std::string& filename);

        bool _ready_to_update = false;
        bool _ready_to_draw = false;
    };
}

#endif
