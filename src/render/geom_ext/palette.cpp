#include "palette.h"
#include "drawable.h"

#include <pybind11/embed.h>

using namespace std;
namespace py = pybind11;


namespace RenderSpace {
    void MeshPalette::colorize(
        uint32_t drawable_id,
        const std::vector<float>& value_per_vertex,
        const std::string& style
    ) {
        py::scoped_interpreter guard{};
        // py::module_ py_render = py::module_::import("render");
        // py::object py_colorize = py_render.attr("colorize");
        // py_colorize(drawable_id, value_per_vertex, style);
    }

    void MeshPalette::show_curvature(
        uint32_t drawable_id,
        const std::string& curvature_type,
        const std::string& style
    ) {
        py::scoped_interpreter guard{};


    }

}