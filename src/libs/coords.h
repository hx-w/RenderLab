#ifndef COORDS_H
#define COORDS_H

#include <cmath>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef double Scalar;

inline bool almostZero(Scalar x) { return (x < 1e-60 && x > -1e-60); }
inline bool almostEqual(Scalar x, Scalar y) { return almostZero(x - y); }

template <typename T>
class Coords {
public:
    Coords() { pt[0] = pt[1] = pt[2] = 0.0; }
    Coords(T x, T y, T z) { pt[0] = x; pt[1] = y; pt[2] = z; }
    Coords(T num) { pt[0] = pt[1] = pt[2] = num; }
    Coords(const Coords& c) { pt[0] = c.pt[0]; pt[1] = c.pt[1]; pt[2] = c.pt[2]; }
    Coords(const T* p) { pt[0] = p[0]; pt[1] = p[1]; pt[2] = p[2]; }
    ~Coords() = default;

    Coords& operator=(const Coords& c) {
        pt[0] = c.pt[0]; pt[1] = c.pt[1]; pt[2] = c.pt[2];
        return *this;
    }
    operator T*() { return pt; }
    T& _x() { return pt[0]; }
    T& _y() { return pt[1]; }
    T& _z() { return pt[2]; }

    T x() const { return pt[0]; }
    T y() const { return pt[1]; }
    T z() const { return pt[2]; }

    T& operator[] (int i) { return ((T&)((T*)this)[i]); }
    const T& operator[] (int i) const { return ((T&)((T*)this)[i]); }

    Coords operator+(const Coords& c) const {
        return Coords(pt[0] + c.pt[0], pt[1] + c.pt[1], pt[2] + c.pt[2]);
    }
    Coords operator-(const Coords& c) const {
        return Coords(pt[0] - c.pt[0], pt[1] - c.pt[1], pt[2] - c.pt[2]);
    }
    Coords operator*(const Coords& c) const {
        return Coords(pt[0] * c.pt[0], pt[1] * c.pt[1], pt[2] * c.pt[2]);
    }
    Coords operator/(const Coords& c) const {
        return Coords(pt[0] / c.pt[0], pt[1] / c.pt[1], pt[2] / c.pt[2]);
    }
    Coords operator*(const T num) const {
        return Coords(pt[0] * num, pt[1] * num, pt[2] * num);
    }
    Coords operator/(const T num) const {
        return Coords(pt[0] / num, pt[1] / num, pt[2] / num);
    }
    Coords operator-() const {
        return Coords(-pt[0], -pt[1], -pt[2]);
    }
    Coords& operator+= (const Coords& c) {
        pt[0] += c.pt[0]; pt[1] += c.pt[1]; pt[2] += c.pt[2];
        return *this;
    }
    Coords& operator-= (const Coords& c) {
        pt[0] -= c.pt[0]; pt[1] -= c.pt[1]; pt[2] -= c.pt[2];
        return *this;
    }
    Coords& operator*= (const Coords& c) {
        pt[0] *= c.pt[0]; pt[1] *= c.pt[1]; pt[2] *= c.pt[2];
        return *this;
    }
    Coords& operator/= (const Coords& c) {
        pt[0] /= c.pt[0]; pt[1] /= c.pt[1]; pt[2] /= c.pt[2];
        return *this;
    }
    Coords& operator*= (const T num) {
        pt[0] *= num; pt[1] *= num; pt[2] *= num;
        return *this;
    }
    Coords& operator/= (const T num) {
        pt[0] /= num; pt[1] /= num; pt[2] /= num;
        return *this;
    }
    bool operator== (const Coords& c) const {
        return almostEqual(pt[0], c.pt[0]) && almostEqual(pt[1], c.pt[1]) && almostEqual(pt[2], c.pt[2]);
    }
    bool operator!= (const Coords& c) const {
        return !(*this == c);
    }
    friend std::ostream &operator<<(std::ostream& output, const Coords& pnt ) { 
        output << "(" << pnt[0] << " " << pnt[1] << " " << pnt[2] << ")";
        return output;            
    }

    Scalar mag2() const { return pt[0] * pt[0] + pt[1] * pt[1] + pt[2] * pt[2]; }
    Scalar mag() const { return sqrt(mag2()); }

    T dist2(const Coords& c) const {
        return (pt[0] - c.pt[0]) * (pt[0] - c.pt[0]) + (pt[1] - c.pt[1]) * (pt[1] - c.pt[1]) + (pt[2] - c.pt[2]) * (pt[2] - c.pt[2]);
    }
    T dist(const Coords& c) const {
        return sqrt(dist2(c));
    }

    Coords cross(const Coords& c) const {
        return Coords(pt[1] * c.pt[2] - pt[2] * c.pt[1], pt[2] * c.pt[0] - pt[0] * c.pt[2], pt[0] * c.pt[1] - pt[1] * c.pt[0]);
    }
    T dot(const Coords& c) const {
        return pt[0] * c.pt[0] + pt[1] * c.pt[1] + pt[2] * c.pt[2];
    }

    static Coords cross(const Coords& a, const Coords& b) {
        return a.cross(b);
    }
    static T dot(const Coords& a, const Coords& b) {
        return a.dot(b);
    }

    Coords abs() const {
        return Coords(fabs(pt[0]), fabs(pt[1]), fabs(pt[2]));
    }

    Coords normalize() const;
    int hash() const;

private:
    T pt[3];
};

typedef Coords<Scalar> Point;
typedef Coords<Scalar> Direction;
typedef Coords<int> Locate;

#endif
