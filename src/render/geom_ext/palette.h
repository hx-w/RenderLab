#pragma once

#include <string>
#include <vector>

namespace RenderSpace {
    class MeshPalette {
    public:
        MeshPalette() = default;
        ~MeshPalette() = default;

        static void colorize(
            uint32_t /* drawable_id */,
            const std::vector<float>& /* value_per_vertex */,
            const std::string& /* style */
        );

        static void show_curvature(
            uint32_t /* drawable_id */,
            const std::string& /* curvature_type */,
            const std::string& /* style */
        );
    };
}