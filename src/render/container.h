/**
 *  renderer -> container
 *  service  -> container
 */

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <any>
#include <geom_types.h>

namespace RenderSpace {
    // !!!important 前向声明 只能用作指针定义
    class Shader;
    class DrawableBase;

    using DrawableID = uint32_t;
    using DrawableHub = std::unordered_map<
        DrawableID,
        std::shared_ptr<DrawableBase>
    >;
    using ShaderHub = std::unordered_map<
        std::string,
        std::shared_ptr<Shader>
    >;

    class RenderContainer {
    public:
        RenderContainer();
        ~RenderContainer();

        void draw_all();

        void update_all();

        ShaderHub& shaders() { return m_shaders; }

        // methods
        DrawableID add_drawable(std::shared_ptr<DrawableBase>);
        bool remove_drawable(DrawableID);
        /// @brief properties:
        ///     - "visible" : bool
        ///     - "shade_mode": uint32_t
        ///     - "shader": Shader
        ///     - "model_transf": Mat4
        bool set_drawable_property(DrawableID, const std::string&, const std::any&);

        std::shared_ptr<DrawableBase> get_drawable_inst(DrawableID);

        /// pick points on triangles
        bool pickcmd(
            geometry::Ray&&,
            std::vector<DrawableID>&, std::vector<geometry::Vector3f>&, std::vector<geometry::Vector3f>&,
            glm::mat4&,
            bool = false
        );

		/// pick vertex indices
        bool pickcmd(
            geometry::Ray&&,
            DrawableID& /* drawable id */, uint32_t& /* vertices id */,
            glm::mat4&,
            float /* threhold */ = 1e-3
        );

    private:
        void _setup();

    private:
        std::mutex m_mutex;

    private:
        ShaderHub m_shaders;
        DrawableHub m_drawables;
    };
}  // namespace RenderSpace