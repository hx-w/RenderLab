#pragma once

#include <string>
#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    typedef std::pair<glm::vec3, glm::vec3> AABB; // min, max
    typedef std::pair<int, int> OrderedEdge; // v1, v2; v1<v2

    typedef glm::vec3 Direction;
    typedef glm::vec3 Point;

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
}