#ifndef GEOM_DRAWABLE_H
#define GEOM_DRAWABLE_H

#include <mutex>
#include <vector>
#include <atomic>
#include <string>

#include <geom_types.h>

#include "../shader.h"

namespace RenderSpace {
    enum GeomType {
        GeomTypePoint = 0x00,
        GeomTypeArrow = 0x01,
        GeomTypeMesh = 0x02,
    };

    struct VertexPrimitive {
        VertexPrimitive() = default;
        VertexPrimitive(const geometry::Vector3f& pos, const geometry::Vector3f& clr, const geometry::Vector3f& nml) :
            Position(pos), Color(clr), Normal(nml) { }
        geometry::Vector3f Position;
        geometry::Vector3f Color;
        geometry::Vector3f Normal;
    };

    // base class
    class DrawableBase {
    public:
        DrawableBase() = default;
        ~DrawableBase();


        /**
         * get_ready -> update -> draw
         */

        void draw();

        void update();

        void get_ready();

        Shader& _shader() { return m_shader; }

        GeomType _type() { return m_type; }

        geometry::Vector3f& _offset() { return m_offset; }

        uint32_t& _shade_mode() { return m_shade_mode; }

    protected:
        void _init_buffer();
        virtual void _draw() = 0;
        virtual void _update() = 0;

    protected:
        uint32_t m_vao;
        uint32_t m_vbo;
        uint32_t m_ebo;

        uint32_t m_shade_mode;  // wireframe or solid
        geometry::Vector3f m_offset;

        Shader m_shader;
        GeomType m_type;
 
    protected:
        std::atomic<bool> _ready_to_update = false;
        std::atomic<bool> _ready_to_draw = false;
    
    protected:
        std::mutex m_mutex;
    };

    // ------------------------------
    class NewMeshDrawable : public DrawableBase {
    public:
        // copy from geometry::mesh
        NewMeshDrawable(geometry::Mesh&, geometry::Vector3f clr);

        void _update() override;
    
        void _draw() override;

    private:
        std::vector<VertexPrimitive> m_vertices;
        std::vector<geometry::Vector3u> m_faces;
    };

}

#endif
