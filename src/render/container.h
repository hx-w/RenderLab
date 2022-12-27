/**
 *  renderer -> container
 *  service  -> container
 */

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "mesh/elements.h"

namespace RenderSpace {
    class RenderContainer {
    public:
        RenderContainer();
        ~RenderContainer();

        void draw_all();

    private:
        void _setup();

    private:
        std::unordered_map<uint32_t, std::shared_ptr<MeshDrawable> > m_meshes;
    };
}  // namespace RenderSpace