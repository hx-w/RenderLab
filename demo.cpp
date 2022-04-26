#include <iostream>
#include <fstream>
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

    double x, y, z;
};

struct SuperPoint {
    SuperPoint() {}
    SuperPoint(Point pnt, double u, double v): pnt(pnt), u(u), v(v) {}
    ~SuperPoint() {}
    SuperPoint(const SuperPoint& sp): pnt(sp.pnt), u(sp.u), v(sp.v) {}
    SuperPoint operator=(const SuperPoint& sp) {
        pnt = sp.pnt, u = sp.u, v = sp.v;
        return *this;
    }

    Point pnt;
    double u;
    double v;
};

struct NURBS_Surface {
    NURBS_Surface() {}
    ~NURBS_Surface() {}
    // u方向点个数-1, v方向点个数-1, u阶数, v阶数
    int maxU, maxV, uOrder, vOrder;
    vector<vector<double> > weight; // 权重
    vector<vector<Point> > pnts;
    vector<vector<double> > T_u; // u 节点矢量
    vector<vector<double> > T_v; // v 节点矢量
};

NURBS_Surface face1, face2, face3, face4;


void GetKnotVector(NURBS_Surface& face, vector<double>& T, int nCount, int num,int order, bool bU)//哈德利-贾德算法获取节点矢量数组
{
    T.clear();
    for (int i = 0; i <= order + num + 1; ++i) {
        if (i <= order) T.push_back(0.0);
        else if (i >= num + 1 && i <= num + order + 1) T.push_back(1.0);
        else if (i >= order + 1 && i <= num) {
            // 计算num-order个内节点
            double sum = 0.0;
            for(int j = order + 1; j <= i; ++j) { 
                double numerator = 0.0;//计算分子
                for (int loop = j - order; loop <= j - 1; ++loop) {
                    if(bU)//选择计算节点矢量U还是计算节点矢量V
                        numerator += (face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)*(face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)+(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y)*(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y);
                    else
                        numerator += (face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)*(face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)+(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y)*(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y);
                }
                double denominator = 0.0;//计算分母
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

double BasisFunctionValue(double t, int i, int order, const vector<double>& T) //计算B样条基函数
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

void handleInput(const string& filename, NURBS_Surface& face) {
    FILE *infile = fopen(filename.c_str(), "r");
    double x, y, z;
    fscanf(infile, "%d %d %d %d", &face.maxV, &face.maxU, &face.uOrder, &face.vOrder);
    // read weights
    for (int i = 0; i < face.maxV + 1; ++i) {
        vector<double> temp;
        for (int j = 0; j < face.maxU + 1; ++j) {
            fscanf(infile, "%lf", &x);
            // infile >> x;
            temp.push_back(x);
        }
        face.weight.push_back(temp);
    }
    // read pnts
    for (int i = 0; i < face.maxV + 1; ++i) {
        vector<Point> temp;
        for (int j = 0; j < face.maxU + 1; ++j) {
            fscanf(infile, "%lf %lf %lf", &x, &y, &z);
            temp.push_back(Point(x, y, z));
        }
        face.pnts.push_back(temp);
    }
    fclose(infile);
}

const Point getPntByUV(NURBS_Surface& face, double v, double u) {
    // face1
    face.T_u.clear();
    face.T_v.clear();
    for (int i = 0; i < face.maxV + 1; ++i) {
        face.T_u.push_back(vector<double>());
        GetKnotVector(face, face.T_u[i], i, face.maxU, face.uOrder, true);
    }
    for (int i = 0; i < face.maxU + 1; ++i) {
        face.T_v.push_back(vector<double>());
        GetKnotVector(face, face.T_v[i], i, face.maxV, face.vOrder, false);
    }
    double weight = 0.0;    
    Point pnt(0, 0, 0);

    for (int i = 0; i < face.maxV + 1; ++i) {
        for (int j = 0; j < face.maxU + 1; ++j) {
            double BValueU = BasisFunctionValue(u, j, face.uOrder, face.T_u[i]);
            double BValueV = BasisFunctionValue(v, i, face.vOrder, face.T_v[j]);
            pnt += face.pnts[i][j] * face.weight[i][j] * BValueU * BValueV;
            weight += face.weight[i][j] * BValueU * BValueV;
        }
    }
    pnt /= weight;
    return pnt;
}

double getDistance(const Point& p1, const Point& p2) {
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + \
                (p2.y - p1.y) * (p2.y - p1.y) + \
                (p2.z - p1.z) * (p2.z - p1.z));
}

#define ESP 1e-6

int main() {
    handleInput("face-1.txt", face1);
    handleInput("face-2.txt", face2);
    handleInput("face-3.txt", face3);
    handleInput("face-4.txt", face4);

    printf("\n输入曲面-1的u v(空格分隔): ");
    double u, v = 0.0;
    scanf("%lf %lf", &u, &v);
    printf("步长step: ");
    double step = 0.01;
    scanf("%lf", &step);
    cout << endl;

    Point pivot = getPntByUV(face1, v, u);

    double f2minDis = 0x00FFFFF;
    SuperPoint f2minSp;
    double f3minDis = 0x00FFFFF;
    SuperPoint f3minSp;
    double f4minDis = 0x00FFFFF;
    SuperPoint f4minSp;
    for (double v = 0.0; v <= 1.0; v += step) {
        for (double u = 0.0; u <= 1.0; u += step) {
            Point _f2p = getPntByUV(face2, v, u);
            double dis = getDistance(pivot, _f2p);
            if (dis < f2minDis) {
                f2minDis = dis;
                f2minSp = SuperPoint(_f2p, u, v);
            }
            Point _f3p = getPntByUV(face3, v, u);
            dis = getDistance(pivot, _f3p);
            if (dis < f3minDis) {
                f3minDis = dis;
                f3minSp = SuperPoint(_f3p, u, v);
            }
            Point _f4p = getPntByUV(face4, v, u);
            dis = getDistance(pivot, _f4p);
            if (dis < f4minDis) {
                f4minDis = dis;
                f4minSp = SuperPoint(_f4p, u, v);
            }
        }
    }

    SuperPoint minSp;
    double minDis = min(min(f2minDis, f3minDis), f4minDis);
    string minFace;
    if (minDis == f2minDis) {
        minSp = f2minSp;
        minFace = "曲面-2";
    } else if (minDis == f3minDis) {
        minSp = f3minSp;
        minFace = "曲面-3";
    } else {
        minSp = f4minSp;
        minFace = "曲面-4";
    }
    
    cout << "曲面-1目标点" << endl;
    cout << "[u, v]: " << u << ", " << v << endl;
    cout << "[point]: " << pivot.x << ", " << pivot.y << ", " << pivot.z << endl;
    cout << "-----------------" << endl;

    cout << "曲面-2最近点" << endl;
    cout << "[distance]: " << f2minDis << endl;
    cout << "[u, v]: " << f2minSp.u << ", " << f2minSp.v << endl;
    cout << "[point]: " << f2minSp.pnt.x << ", " << f2minSp.pnt.y << ", " << f2minSp.pnt.z << endl;
    cout << "-----------------" << endl;

    cout << "曲面-3最近点" << endl;
    cout << "[distance]: " << f3minDis << endl;
    cout << "[u, v]: " << f3minSp.u << ", " << f3minSp.v << endl;
    cout << "[point]: " << f3minSp.pnt.x << ", " << f3minSp.pnt.y << ", " << f3minSp.pnt.z << endl;
    cout << "-----------------" << endl;

    cout << "曲面-4最近点" << endl;
    cout << "[distance]: " << f4minDis << endl;
    cout << "[u, v]: " << f4minSp.u << ", " << f4minSp.v << endl;
    cout << "[point]: " << f4minSp.pnt.x << ", " << f4minSp.pnt.y << ", " << f4minSp.pnt.z << endl;
    cout << "-----------------" << endl;

    cout << "↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓" << endl << "最近点" << endl;
    cout << "[distance]: " << minDis << endl;
    cout << "[u, v]: " << minSp.u << ", " << minSp.v << endl;
    cout << "[point]: " << minSp.pnt.x << ", " << minSp.pnt.y << ", " << minSp.pnt.z << endl;
    cout << "[face]: " << minFace << endl << endl;

    system("pause");
    return 0;
}