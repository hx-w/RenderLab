#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

struct Point {
    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    ~Point() {}
    Point(const Point& p): x(p.x), y(p.y), z(p.z) {}

    bool operator==(const Point& p) {
        return p.x == x && p.y == y && p.z == z;
    }

    Point operator+(const Point& p) const {
        return Point(x + p.x, y + p.y, z + p.z);
    }

    Point operator*(const double& num) const {
        return Point(x * num, y * num, z * num);
    }

    Point* operator+=(const Point& p) {
        x += p.x; y += p.y; z += p.z;
        return this;
    }

    Point* operator/=(const double& num) {
        x /= num; y /= num; z /= num;
        return this;
    }

    friend ostream &operator<<(ostream& output, const Point& pnt ) { 
        output << "(" << pnt.x << " " << pnt.y << " " << pnt.z << ")";
        return output;            
    }

    double x, y, z;
};

struct SuperPoint: public Point {
    SuperPoint() {}
    SuperPoint(Point pnt, double u, double v): u(u), v(v) {
        x = pnt.x, y = pnt.y, z = pnt.z;
    }
    ~SuperPoint() {}
    SuperPoint(const SuperPoint& sp): u(sp.u), v(sp.v) {
        x = sp.x, y = sp.y, z = sp.z;
    }
    SuperPoint operator=(const SuperPoint& sp) {
        x = sp.x, y = sp.y, z = sp.z, u = sp.u, v = sp.v;
        return *this;
    }
    int samelike(const SuperPoint& sp) {
        return int(u == sp.u) + int(v == sp.v);
    }

    double u;
    double v;
};

struct NURBS_Surface {
    NURBS_Surface() {}
    ~NURBS_Surface() {}
    int maxU, maxV, uOrder, vOrder;
    vector<vector<double> > weight;
    vector<vector<Point> > pnts;
    vector<vector<double> > T_u; // u vector
    vector<vector<double> > T_v; // v vector
};

static void GetKnotVector(NURBS_Surface& face, vector<double>& T, int nCount, int num,int order, bool bU)//哈德利-贾德算法获取节点矢量数组
{
    T.clear();
    for (int i = 0; i <= order + num + 1; ++i) {
        if (i <= order) T.push_back(0.0);
        else if (i >= num + 1 && i <= num + order + 1) T.push_back(1.0);
        else if (i >= order + 1 && i <= num) {
            double sum = 0.0;
            for(int j = order + 1; j <= i; ++j) { 
                double numerator = 0.0;
                for (int loop = j - order; loop <= j - 1; ++loop) {
                    if(bU)
                        numerator += (face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)*(face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)+(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y)*(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y);
                    else
                        numerator += (face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)*(face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)+(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y)*(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y);
                }
                double denominator = 0.0;
                for (int loop1 = order + 1; loop1 <= num + 1; ++loop1) {
                    for(int loop2 = loop1 - order; loop2 <= loop1 - 1; ++loop2) {
                        if(bU)
                            denominator+=(face.pnts[nCount][loop2].x-face.pnts[nCount][loop2-1].x)*(face.pnts[nCount][loop2].x-face.pnts[nCount][loop2-1].x)+(face.pnts[nCount][loop2].y-face.pnts[nCount][loop2-1].y)*(face.pnts[nCount][loop2].y-face.pnts[nCount][loop2-1].y);
                        else
                            denominator+=(face.pnts[loop2][nCount].x-face.pnts[loop2-1][nCount].x)*(face.pnts[loop2][nCount].x-face.pnts[loop2-1][nCount].x)+(face.pnts[loop2][nCount].y-face.pnts[loop2-1][nCount].y)*(face.pnts[loop2][nCount].y-face.pnts[loop2-1][nCount].y);
                    }
                } 
                sum += numerator / denominator;			
            }
            T.push_back(sum);
        }
        else {
            cout << "error" << endl;
        }
    }
}

static double BasisFunctionValue(double t, int i, int order, const vector<double>& T) //计算B样条基函数
{
	double value1, value2, value;
	if (order == 0) {
		if(t >= T[i] && t < T[i + 1])
			return 1.0;
		else
			return 0.0;
	}
	if (order > 0) {
		if(t < T[i] || t > T[i + order + 1]) {
			return 0.0;
        }
		else {
			double coffcient1, coffcient2;//凸组合系数1，凸组合系数2
			double denominator = 0.0;//分母
			denominator = T[i + order] - T[i];//递推公式第一项分母
			if(denominator == 0.0)//约定0/0
				coffcient1 = 0.0;
			else
				coffcient1=(t - T[i])/denominator;
			denominator=T[i + order + 1] - T[i + 1]; //递推公式第二项分母
			if(0.0 == denominator)//约定0/0
				coffcient2 = 0.0;
			else
				coffcient2 = (T[i + order + 1] - t) / denominator;
			value1 = coffcient1 * BasisFunctionValue(t, i, order - 1, T);//递推公式第一项的值
			value2 = coffcient2 * BasisFunctionValue(t, i + 1, order - 1, T);//递推公式第二项的值
			value = value1 + value2;//基函数的值
		}
	}
	return value;
}