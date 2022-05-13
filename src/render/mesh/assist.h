#ifndef ASSIST_ELEMENT_H
#define ASSIST_ELEMENT_H

#include "drawable.h"

namespace RenderSpace {
    class AssistPoint : public Drawable {
    public:
        AssistPoint() = default;
        AssistPoint(const std::string& name, DrawableType type):
            Drawable(name, type) {}
        ~AssistPoint();
        AssistPoint(const AssistPoint&) = default;
        AssistPoint& operator=(const AssistPoint&) = default;

        // void draw() override;
        // void sync() override;
    };
}


#endif
