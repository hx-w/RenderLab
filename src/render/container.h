/**
 *  renderer -> container
 *  service  -> container
 */

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include "geom_ext/drawable.h"
#include "shader.h"

namespace RenderSpace {
    using DrawableID = uint32_t;
    using DrawableHub = std::unordered_map<
        DrawableID,
        std::shared_ptr<DrawableBase>
    >;
    using ShaderHub = std::unordered_map<
        std::string,
        Shader
    >;

    class RenderContainer {
    public:
        RenderContainer();
        ~RenderContainer();

        void draw_all();

        void update_all();

        ShaderHub& shaders() { return m_shaders; }

        DrawableID add_drawable(std::shared_ptr<DrawableBase>);

    private:
        void _setup();

    private:
        std::mutex m_mutex;

    private:
        ShaderHub m_shaders;
        DrawableHub m_drawables;
    };
}  // namespace RenderSpace