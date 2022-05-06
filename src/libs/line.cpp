#include "line.h"

Direction get_normal(const Point& p1, const Point& p2, const Point& p3) {
    Direction d1 = p2 - p1;
    Direction d2 = p3 - p1;
    return d1.cross(d2);
}
