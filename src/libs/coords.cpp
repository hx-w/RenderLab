#include "coords.h"

int Coords::hash() const {
    int hid = 0;
    for (unsigned int i = 0; i < sizeof(Coords) / sizeof(int); ++i) {
        hid = hid ^ ((const int*)this)[i];
    }
    return hid;
}

Coords Coords::normalize() const {
    Scalar m = mag();
    if (almostZero(m)) {
        return Coords(0.0, 0.0, 0.0);
    }
    return *this / mag();
}