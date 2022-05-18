/**
 * 文本渲染管理器
 */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include "character.h"

namespace RenderSpace {
    struct TextSegment {
        std::string Text;
        glm::vec3 Color;
        uint32_t Size;
    };

    typedef std::vector<TextSegment> RenderLine;

    // 分为四个区域，分别是左上角，右上角，右下角，左下角
    enum BoxRegion {
        BOX_LEFT_TOP = 0,
        BOX_RIGHT_TOP,
        BOX_RIGHT_BOTTOM,
        BOX_LEFT_BOTTOM,
    };

    class TextBox {
    public:
        TextBox() = default;
        TextBox(BoxRegion region, Shader shader):
            m_region(region), m_shader(shader), m_width(0), m_height(0) {}
        ~TextBox() { clear(); };
        TextBox& operator=(const TextBox&);

        void add_text(RenderLine&& rtext);
        void update_text(int index, RenderLine&& rtext);
        void delete_text(int index);
        void clear();
        void draw(std::shared_ptr<TextRenderer> text_renderer, uint32_t win_width, uint32_t win_height);

    private:
        void _refresh_size();

    private:
        BoxRegion m_region;
        Shader m_shader;
        std::vector<RenderLine> m_lines;
        uint32_t m_width; // 包围盒
        uint32_t m_height;


        std::mutex m_mutex;
    };

    class TextService {
    public:
        TextService() = delete;
        TextService(const TextService&) = delete;
        TextService(Shader& shader);
        ~TextService();

        void update_window_size(uint32_t width, uint32_t height);
        void draw();
    
    private:
        std::string _time_now() const;
        uint32_t _fps(); // 需要每帧调用

    private:
        std::shared_ptr<TextRenderer> m_text_render;

        std::map<BoxRegion, TextBox> m_boxes;

        uint32_t m_width;  // 窗口大小
        uint32_t m_height;

        std::chrono::system_clock::time_point m_timepoint;

        int m_counter = 0;
    };
}

#endif
