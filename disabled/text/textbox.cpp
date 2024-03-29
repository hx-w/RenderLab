#include "textbox.h"
#include <ctime>
#include <algorithm>

using namespace std;
namespace RenderSpace {
    void TextBox::clear() {
        lock_guard<mutex> lock(m_mutex);
        // deep free
        vector<RenderLine>().swap(m_lines);
        m_width = 0;
        m_height = 0;
    }

    TextBox& TextBox::operator=(const TextBox& tb) {
        m_lines.assign(tb.m_lines.begin(), tb.m_lines.end());
        m_shader = tb.m_shader;
        m_width = tb.m_width;
        m_height = tb.m_height;
        m_region = tb.m_region;
        return *this;
    }

    void TextBox::_refresh_size() {
        uint32_t _width = 0;
        uint32_t _height = 0;
        for (const auto& line : m_lines) {
            uint32_t _cwidth = 0;
            for (const auto& seg : line.Segments) {
                _cwidth += seg.Size * seg.Text.length();
                _height += seg.Size;
            }
            _width = max(_width, _cwidth);
        }
        m_width = _width / 1.7;
        m_height = _height;
    }

    void TextBox::add_text(RenderLine&& rtext) {
        lock_guard<mutex> lock(m_mutex);
        m_lines.push_back(rtext);
        uint32_t _width = 0;
        uint32_t _height = 0;
        for (const auto& seg : rtext.Segments) {
            _width += seg.Size * seg.Text.length(); 
            _height += seg.Size;
        }
        m_width = std::max(m_width, _width) / 1.7;
        m_height = std::max(m_height, _height);
    }

    void TextBox::update_text(int index, RenderLine&& rtext) {
        lock_guard<mutex> lock(m_mutex);
        if (index < 0 || index >= m_lines.size()) {
            return;
        }
        m_lines[index].Segments.clear();
        m_lines[index].Segments.assign(rtext.Segments.begin(), rtext.Segments.end());
        m_lines[index].frame_remain = rtext.frame_remain;
        _refresh_size();
    }

    void TextBox::delete_text(int index) {
        lock_guard<mutex> lock(m_mutex);
        if (index < 0 || index >= m_lines.size()) {
            return;
        }
        m_lines.erase(m_lines.begin() + index);
        _refresh_size();
    }

    void TextBox::draw(shared_ptr<TextRenderer> text_renderer, uint32_t win_width, uint32_t win_height) {
        lock_guard<mutex> lock(m_mutex);
        // screen zero at left-bottom
        // get text box left top pivot
        glm::vec2 _pivot = glm::vec2(0.0f, 0.0f);
        float _shift = 25.0f;
        switch (m_region) {
        case BOX_LEFT_TOP:
            _pivot = glm::vec2(_shift, win_height - _shift);
            break;
        case BOX_RIGHT_TOP:
            _pivot = glm::vec2(win_width - _shift - m_width, win_height - _shift);
            break;
        case BOX_RIGHT_BOTTOM:
            _pivot = glm::vec2(win_width - _shift - m_width, _shift);
            break;
        case BOX_LEFT_BOTTOM:
            _pivot = glm::vec2(_shift, _shift);
            break;
        default:
            // error region type
            break;
        }
        float xpos = _pivot.x;
        float ypos = _pivot.y;
        uint32_t _last_h = 0;
        for (auto& line : m_lines) {
            xpos = _pivot.x;
            ypos -= _last_h + 3.0; // disturb
            _last_h = 0;
            float _last_w = 0.0f;
            for (auto& segment : line.Segments) {
                xpos += _last_w;
                float _scale = float(segment.Size) / text_renderer->get_csize();
                text_renderer->render_text(segment.Text, xpos, ypos, _scale, segment.Color);
                _last_h = std::max(_last_h, segment.Size);
                _last_w += segment.Size * segment.Text.length() * 1.0 / 1.7;
            }
            if (line.frame_remain > 0) {
                line.frame_remain--;
            }
        }
        for (auto iter = m_lines.begin(); iter != m_lines.end();) {
            if (!iter->frame_remain) {
                iter = m_lines.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    TextService::TextService(Shader& shader) {
        m_text_render = make_shared<TextRenderer>(shader);
        m_boxes[BOX_LEFT_BOTTOM] = TextBox(BOX_LEFT_BOTTOM, shader);
        m_boxes[BOX_LEFT_TOP] = TextBox(BOX_LEFT_TOP, shader);
        m_boxes[BOX_RIGHT_BOTTOM] = TextBox(BOX_RIGHT_BOTTOM, shader);
        m_boxes[BOX_RIGHT_TOP] = TextBox(BOX_RIGHT_TOP, shader);

        // 左下角时间显示
        m_boxes[BOX_LEFT_BOTTOM].add_text(RenderLine(TextSegment{_time_now(), glm::vec3(0.7f, 0.7f, 0.7f), 20}, -1));
        m_boxes[BOX_LEFT_TOP].add_text(RenderLine(TextSegment{to_string(_fps()) + " FPS", glm::vec3(0.7f, 0.7f, 0.7f), 16}, -1));
        
        m_timepoint = chrono::system_clock::now();
    }

    TextService::~TextService() {
        m_boxes.clear();
    }

    string TextService::_time_now() const {
        time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        char _buffer[64] = "\0";
        strftime(_buffer, sizeof(_buffer), "%H:%M:%S", localtime(&now));
        return move(string(_buffer));
    }

    uint32_t TextService::_fps() {
        auto now = chrono::system_clock::now();
        chrono::duration<double, std::milli> tm = now - m_timepoint;
        m_timepoint = now;
        return static_cast<uint32_t>(1 / (tm.count() / 1000.0f));
    }

    void TextService::update_window_size(uint32_t w, uint32_t h) {
        m_width = w;
        m_height = h;
    }

    void TextService::draw() {
        auto _fr = _fps();
        if ((m_counter++) % 60 == 0 && m_counter > 1) {
            m_boxes[BOX_LEFT_BOTTOM].update_text(0, RenderLine(TextSegment{_time_now(), glm::vec3(0.7f, 0.7f, 0.7f), 22}, -1));
            m_boxes[BOX_LEFT_TOP].update_text(0, RenderLine(TextSegment{to_string(_fr) + " FPS", glm::vec3(0.7f, 0.7f, 0.7f), 16}, -1));
            m_counter = 0;
        }
        for (auto& [_region, _box] : m_boxes) {
            _box.draw(m_text_render, m_width, m_height);
        }
    }

    void TextService::clear_text(BoxRegion region) {
        m_boxes[region].clear();
    }

    void TextService::add_text(BoxRegion region, RenderLine&& rtext) {
        m_boxes[region].add_text(move(rtext));
    }

    void TextService::update_text(BoxRegion region, int index, RenderLine&& rtext) {
        m_boxes[region].update_text(index, move(rtext));
    }

    void TextService::delete_text(BoxRegion region, int index) {
        m_boxes[region].delete_text(index);
    }
}
