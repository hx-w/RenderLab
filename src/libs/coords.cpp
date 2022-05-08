#include "coords.h"

template <typename T>
int Coords<T>::hash() const {
    int hid = 0;
    for (unsigned int i = 0; i < sizeof(Coords<T>) / sizeof(int); ++i) {
        hid = hid ^ ((const int*)this)[i];
    }
    return hid;
}

template <typename T>
Coords<T> Coords<T>::normalize() const {
    T m = mag();
    if (almostZero(m)) {
        return Coords(0, 0, 0);
    }
    return *this / mag();
}


template class Coords<Scalar>;
template class Coords<int>;
