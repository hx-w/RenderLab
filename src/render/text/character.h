#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <map>
#include "../shader.hpp"
#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    struct Character {
        unsigned int TextureID;   // ID handle of the glyph texture
        glm::ivec2 Size;    // Size of glyph
        glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
        unsigned int Advance;    // Horizontal offset to advance to next glyph
    };

    class TextRenderer {
    public:
        TextRenderer() = delete;
        TextRenderer(Shader& shader);
        ~TextRenderer();

        void render_text(const std::string& text, float x, float y, float scale, glm::vec3 color);

    private:
        void _load_characters();

    private:
        std::map<char, Character> Characters;
        unsigned int m_vao, m_vbo;
        Shader m_shader;
    };
}
#endif
