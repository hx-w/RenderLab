#include <omp.h>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

struct Point {
    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    ~Point() {}
    Point(const Point& p) : x(p.x), y(p.y), z(p.z) {}

    Point operator=(const Point& p) {
        x = p.x, y = p.y, z = p.z;
        return *this;
    }

    bool operator==(const Point& p) { return p.x == x && p.y == y && p.z == z; }

    Point operator+(const Point& p) const {
        return Point(x + p.x, y + p.y, z + p.z);
    }

    Point operator*(const double& num) const {
        return Point(x * num, y * num, z * num);
    }

    Point* operator+=(const Point& p) {
        x += p.x;
        y += p.y;
        z += p.z;
        return this;
    }

    Point* operator/=(const double& num) {
        x /= num;
        y /= num;
        z /= num;
        return this;
    }

    double x, y, z;
};

struct SuperPoint {
    SuperPoint() { addon = 0x00FFFF; }
    SuperPoint(Point pnt, double u, double v, double addon=0x00FFFF) : pnt(pnt), u(u), v(v), addon(addon) {}
    ~SuperPoint() {}
    SuperPoint(const SuperPoint& sp) : pnt(sp.pnt), u(sp.u), v(sp.v), addon(sp.addon) {}
    SuperPoint operator=(const SuperPoint& sp) {
        pnt = sp.pnt, u = sp.u, v = sp.v;
        addon = sp.addon;
        return *this;
    }

    bool operator<(const SuperPoint& sp) const {
        return addon < sp.addon;
    }

    Point pnt;
    double u;
    double v;
    double addon;
};

struct NURBS_Surface {
    NURBS_Surface() {}
    ~NURBS_Surface() {}
    // u方向点个数-1, v方向点个数-1, u阶数, v阶数
    int maxU, maxV, uOrder, vOrder;
    vector<vector<double> > weight;  // 权重
    vector<vector<Point> > pnts;
    vector<vector<double> > T_u;  // u 节点矢量
    vector<vector<double> > T_v;  // v 节点矢量
};

NURBS_Surface face1, face2, face3, face4;

void GetKnotVector(NURBS_Surface& face,
                   vector<double>& T,
                   int nCount,
                   int num,
                   int order,
                   bool bU)  //哈德利-贾德算法获取节点矢量数组
{
    T.clear();
    for (int i = 0; i <= order + num + 1; ++i) {
        if (i <= order)
            T.push_back(0.0);
        else if (i >= num + 1 && i <= num + order + 1)
            T.push_back(1.0);
        else if (i >= order + 1 && i <= num) {
            // 计算num-order个内节点
            double sum = 0.0;
            for (int j = order + 1; j <= i; ++j) {
                double numerator = 0.0;  //计算分子
                for (int loop = j - order; loop <= j - 1; ++loop) {
                    if (bU)  //选择计算节点矢量U还是计算节点矢量V
                        numerator += (face.pnts[nCount][loop].x -
                                      face.pnts[nCount][loop - 1].x) *
                                         (face.pnts[nCount][loop].x -
                                          face.pnts[nCount][loop - 1].x) +
                                     (face.pnts[nCount][loop].y -
                                      face.pnts[nCount][loop - 1].y) *
                                         (face.pnts[nCount][loop].y -
                                          face.pnts[nCount][loop - 1].y);
                    else
                        numerator += (face.pnts[loop][nCount].x -
                                      face.pnts[loop - 1][nCount].x) *
                                         (face.pnts[loop][nCount].x -
                                          face.pnts[loop - 1][nCount].x) +
                                     (face.pnts[loop][nCount].y -
                                      face.pnts[loop - 1][nCount].y) *
                                         (face.pnts[loop][nCount].y -
                                          face.pnts[loop - 1][nCount].y);
                }
                double denominator = 0.0;  //计算分母
                for (int loop1 = order + 1; loop1 <= num + 1; ++loop1) {
                    for (int loop2 = loop1 - order; loop2 <= loop1 - 1;
                         ++loop2) {
                        if (bU)
                            denominator +=
                                (face.pnts[nCount][loop2].x -
                                 face.pnts[nCount][loop2 - 1].x) *
                                    (face.pnts[nCount][loop2].x -
                                     face.pnts[nCount][loop2 - 1].x) +
                                (face.pnts[nCount][loop2].y -
                                 face.pnts[nCount][loop2 - 1].y) *
                                    (face.pnts[nCount][loop2].y -
                                     face.pnts[nCount][loop2 - 1].y);
                        else
                            denominator +=
                                (face.pnts[loop2][nCount].x -
                                 face.pnts[loop2 - 1][nCount].x) *
                                    (face.pnts[loop2][nCount].x -
                                     face.pnts[loop2 - 1][nCount].x) +
                                (face.pnts[loop2][nCount].y -
                                 face.pnts[loop2 - 1][nCount].y) *
                                    (face.pnts[loop2][nCount].y -
                                     face.pnts[loop2 - 1][nCount].y);
                    }
                }
                sum += numerator / denominator;
            }
            T.push_back(sum);
        } else {
            cout << "error" << endl;
        }
    }
}

double BasisFunctionValue(double t,
                          int i,
                          int order,
                          const vector<double>& T)  //计算B样条基函数
{
    double value1, value2, value;
    if (order == 0) {
        if (t >= T[i] && t < T[i + 1])
            return 1.0;
        else
            return 0.0;
    }
    if (order > 0) {
        if (t < T[i] || t > T[i + order + 1]) {
            return 0.0;
        } else {
            double coffcient1, coffcient2;  //凸组合系数1，凸组合系数2
            double denominator = 0.0;       //分母
            denominator = T[i + order] - T[i];  //递推公式第一项分母
            if (denominator == 0.0)             //约定0/0
                coffcient1 = 0.0;
            else
                coffcient1 = (t - T[i]) / denominator;
            denominator = T[i + order + 1] - T[i + 1];  //递推公式第二项分母
            if (0.0 == denominator)                     //约定0/0
                coffcient2 = 0.0;
            else
                coffcient2 = (T[i + order + 1] - t) / denominator;
            value1 = coffcient1 * BasisFunctionValue(t, i, order - 1,
                                                     T);  //递推公式第一项的值
            value2 = coffcient2 * BasisFunctionValue(t, i + 1, order - 1,
                                                     T);  //递推公式第二项的值
            value = value1 + value2;                      //基函数的值
        }
    }
    return value;
}

void handleInput(const string& filename, NURBS_Surface& face) {
    FILE* infile = fopen(filename.c_str(), "r");
    double x, y, z;
    fscanf(infile, "%d %d %d %d", &face.maxV, &face.maxU, &face.uOrder,
           &face.vOrder);
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
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) +
                (p2.z - p1.z) * (p2.z - p1.z));
}

// 预处理 计算uv点阵
void preprocess(NURBS_Surface& face, vector<SuperPoint>& spnts, double step) {
    for (double fv = 0.0; fv <= 1.0; fv += step) {
        for (double fu = 0.0; fu <= 1.0; fu += step) {
            Point pnt = getPntByUV(face, fv, fu);
            spnts.emplace_back(SuperPoint(pnt, fu, fv));
        }
    }
}

double getNearestSuperPoint(vector<SuperPoint>& spnts,
                            const Point& pnt,
                            SuperPoint& nearest) {
    nearest = spnts[0];
    double minDistance = getDistance(pnt, nearest.pnt);
    for (auto& spnt : spnts) {
        double distance = getDistance(pnt, spnt.pnt);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = spnt;
        }
    }
    return minDistance;
}

#define EPS 1e-6

int main() {
    // omp_set_num_threads(4);
#pragma omp declare reduction(mindist: SuperPoint: omp_out=std::min(omp_out,omp_in)) initializer(omp_priv=SuperPoint()) 

#pragma omp sections
{
#pragma omp section
    { handleInput("face-1.txt", face1); }
#pragma omp section
    { handleInput("face-2.txt", face2); }
#pragma omp section
    { handleInput("face-3.txt", face3); }
#pragma omp section
    { handleInput("face-4.txt", face4); }
}
    printf(
        "\n输出文件为同目录下的table."
        "csv，如果有相同名称的文件将会覆盖内容！\n");
    printf(
        "\n程序会以step为步长在face1中将uv分别在[0.0, "
        "1.0]区间遍历点，并以该点为基准，在face2中以同样步长搜索最近点\n例 "
        "步长为0.1，uv方向分别遍历11次，共121种组合");
    printf(
        "\n但由于系统浮点数误差，实际的计算次数有偏差，例如步长为0."
        "05时，uv方向应分别遍历21次，但实际可能只有20次\n");
    printf("\n[输入步长step] ");
    double step = 0.01;
    scanf("%lf", &step);
    cout << endl;
    const int times = floor(1.0 / step) + 1;

    ofstream ofile;
    ofile.open("table.csv", ios::out);
    ofile << "face1 uv, face1 xyz, face2 uv, face2 xyz, face2 L, face3 uv, "
             "face3 xyz, face3 L, face4 uv, face4 xyz, face4 L, min face, min L"
          << endl;

    clock_t stime, etime;
    stime = clock();

    // 预先记录face2, face3, face4中的点阵
    vector<SuperPoint> face2pnts;
    vector<SuperPoint> face3pnts;
    vector<SuperPoint> face4pnts;

#pragma omp parallel sections
{
#pragma omp section
    { preprocess(face2, face2pnts, step); }
#pragma omp section
    { preprocess(face3, face3pnts, step); }
#pragma omp section
    { preprocess(face4, face4pnts, step); }
}
    const int savepoint = 500;  // 计算500次保存一次数据 避免内存溢出

    map<string, int> minFaceCount{{"face2", 0}, {"face3", 0}, {"face4", 0}};
    for (double f1v = 0.0; f1v <= 1.0; f1v += step) {
        for (double f1u = 0.0; f1u <= 1.0; f1u += step) {
            int count = (f1v / step) * times + (f1u / step) + 1;
            if (count % 500 == 0) {
                // 保存数据
                ofile.close();
                ofile.open("table.csv", ios::app);
            }
            printf("\r[processed] %.3lf%%", 100.0 * count / (times * times));
            // 遍历曲面1中的点
            Point pivot = getPntByUV(face1, f1v, f1u);
            SuperPoint f2nearest;
            SuperPoint f3nearest;
            SuperPoint f4nearest;
            double f2minDistance, f3minDistance, f4minDistance;
            #pragma omp parallel sections
            {
            #pragma omp section
                {
                    f2minDistance = getNearestSuperPoint(face2pnts, pivot,
                                                         f2nearest);
                }
            #pragma omp section
                {
                    f3minDistance = getNearestSuperPoint(face3pnts, pivot,
                                                         f3nearest);
                }
            #pragma omp section
                {
                    f4minDistance = getNearestSuperPoint(face4pnts, pivot,
                                                         f4nearest);
                }
            }
            // 写入文件
            double minDistance = min(f2minDistance, min(f3minDistance, f4minDistance));
            string minFace = "face2";
            if ((f3minDistance + EPS) < f2minDistance) {
                minFace = "face3";
                if ((f4minDistance + EPS) < f3minDistance) {
                    minFace = "face4";
                } else {
                    cout << f2minDistance << " " << f3minDistance << endl;
                }
            } else if ((f4minDistance + EPS) < f2minDistance) {
                minFace = "face4";
            }
            minFaceCount[minFace]++;

            ofile << "\"" << f1u << "," << f1v << "\","
                  << "\"" << pivot.x << "," << pivot.y << "," << pivot.z
                  << "\","
                  << "\"" << f2nearest.u << "," << f2nearest.v << "\","
                  << "\"" << f2nearest.pnt.x << "," << f2nearest.pnt.y << ","
                  << f2nearest.pnt.z << "\"," << f2minDistance << ","
                  << "\"" << f3nearest.u << "," << f3nearest.v << "\","
                  << "\"" << f3nearest.pnt.x << "," << f3nearest.pnt.y << ","
                  << f3nearest.pnt.z << "\"," << f3minDistance << ","
                  << "\"" << f4nearest.u << "," << f4nearest.v << "\","
                  << "\"" << f4nearest.pnt.x << "," << f4nearest.pnt.y << ","
                  << f4nearest.pnt.z << "\"," << f4minDistance << ","
                  << "\"" << minFace << "\"" << endl;
        }
    }
    etime = clock();
    double total_sec = double(etime - stime) / CLOCKS_PER_SEC;
    int cost_min = floor(total_sec / 60);
    double cost_sec = total_sec - cost_min * 60;
    cout << endl
         << endl
         << "[耗时] " << cost_min << "min " << cost_sec << "s" << endl;

    cout << "\n最小距离面统计" << endl;

    for (auto& iter : minFaceCount) {
        if (iter.second == 0)
            continue;
        cout << iter.first << ": " << iter.second << "次" << endl;
    }
    cout << endl;

    ofile.close();

    system("pause");
    return 0;
}