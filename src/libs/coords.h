#ifndef COORDS_H
#define COORDS_H

#include <cmath>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef double Scalar;

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

inline bool almostZero(Scalar x) { return (x < 1e-60 && x > -1e-60); }
inline bool almostEqual(Scalar x, Scalar y) { return almostZero(x - y); }

class Coords {
public:
    Coords() { pt[0] = pt[1] = pt[2] = 0.0; }
    Coords(Scalar x, Scalar y, Scalar z) { pt[0] = x; pt[1] = y; pt[2] = z; }
    Coords(double num) { pt[0] = pt[1] = pt[2] = num; }
    Coords(float num) { pt[0] = pt[1] = pt[2] = num; }
    Coords(const Coords& c) { pt[0] = c.pt[0]; pt[1] = c.pt[1]; pt[2] = c.pt[2]; }
    Coords(const double* p) { pt[0] = p[0]; pt[1] = p[1]; pt[2] = p[2]; }
    Coords(const float* p) { pt[0] = p[0]; pt[1] = p[1]; pt[2] = p[2]; }
    ~Coords() = default;

    Coords& operator=(const Coords& c) {
        pt[0] = c.pt[0]; pt[1] = c.pt[1]; pt[2] = c.pt[2];
        return *this;
    }
    operator double*() { return pt; }
    Scalar x() const { return pt[0]; }
    Scalar y() const { return pt[1]; }
    Scalar z() const { return pt[2]; }

    Scalar& operator[] (int i) { return ((Scalar&)((Scalar*)this)[i]); }
    const Scalar& operator[] (int i) const { return ((Scalar&)((Scalar*)this)[i]); }

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
    Coords operator*(const Scalar num) const {
        return Coords(pt[0] * num, pt[1] * num, pt[2] * num);
    }
    Coords operator/(const Scalar num) const {
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
    Coords& operator*= (const Scalar num) {
        pt[0] *= num; pt[1] *= num; pt[2] *= num;
        return *this;
    }
    Coords& operator/= (const Scalar num) {
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

    Scalar dist2(const Coords& c) const {
        return (pt[0] - c.pt[0]) * (pt[0] - c.pt[0]) + (pt[1] - c.pt[1]) * (pt[1] - c.pt[1]) + (pt[2] - c.pt[2]) * (pt[2] - c.pt[2]);
    }
    Scalar dist(const Coords& c) const {
        return sqrt(dist2(c));
    }

    Coords cross(const Coords& c) const {
        return Coords(pt[1] * c.pt[2] - pt[2] * c.pt[1], pt[2] * c.pt[0] - pt[0] * c.pt[2], pt[0] * c.pt[1] - pt[1] * c.pt[0]);
    }
    Coords dot(const Coords& c) const {
        return pt[0] * c.pt[0] + pt[1] * c.pt[1] + pt[2] * c.pt[2];
    }

    Coords abs() const {
        return Coords(fabs(pt[0]), fabs(pt[1]), fabs(pt[2]));
    }

    Coords normalize() const;
    int hash() const;

private:
    Scalar pt[3];
};

#endif
