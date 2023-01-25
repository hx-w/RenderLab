#pragma once

#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include <memory>

#include <geom_types.h>

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

    class Shader;
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

        std::shared_ptr<Shader>& _shader() { return m_shader; }

        GeomType& _type() { return m_type; }

        uint32_t& _shade_mode() { return m_shade_mode; }

        bool& _visible() { return m_visible; }

        std::shared_ptr<glm::quat>& _rot() { return m_rot; }

    protected:
        void _init_buffer();
        virtual void _draw() = 0;
        virtual void _update() = 0;

    protected:
        uint32_t m_vao = 0;
        uint32_t m_vbo = 0;
        uint32_t m_ebo = 0;

        uint32_t m_shade_mode = 0;  // wireframe or solid

        std::shared_ptr<glm::quat> m_rot; // same with xwindow

        std::shared_ptr<Shader> m_shader;
        GeomType m_type;
 
    protected:
        std::atomic<bool> _ready_to_update = false;
        std::atomic<bool> _ready_to_draw = false;
    
        bool m_visible = true;

    protected:
        std::mutex m_mutex;
    };

    // ------------------------------
    class NewMeshDrawable : public DrawableBase {
    public:
        // copy from geometry::mesh
        NewMeshDrawable(geometry::Mesh&, geometry::Vector3f /* color */);

        void _update() override;
    
        void _draw() override;

        decltype(auto) _raw() { return m_raw; }

    private:
        std::vector<VertexPrimitive> m_vertices;
        std::vector<geometry::Vector3u> m_faces;

        std::shared_ptr<geometry::Mesh> m_raw; // stupid
    };

    class ArrowDrawable : public DrawableBase {
    public:
        ArrowDrawable(geometry::Ray&, float /* length */, geometry::Vector3f /* color */);

        void _update() override;

        void _draw() override;

        decltype(auto) _raw() { return m_raw; }

    private:
        std::vector<VertexPrimitive> m_vertices;
        std::vector<unsigned int> m_lines;

        std::shared_ptr<geometry::Ray> m_raw;
    };

}
