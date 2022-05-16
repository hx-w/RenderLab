/**
 * 文本渲染管理器
 */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <memory>
#include "character.h"


namespace RenderSpace {
    class TextBox {
    public:
        TextBox() = delete;
        TextBox(const TextBox&) = delete;
        TextBox(Shader& shader);
        ~TextBox();

        void draw();
    private:
        std::unique_ptr<TextRenderer> m_text_render;
    };
}

#endif
