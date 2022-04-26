#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <map>
using namespace std;

struct Point {
    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    ~Point() {}
    Point(const Point& p): x(p.x), y(p.y), z(p.z) {}

    Point operator=(const Point& p) {
        x = p.x, y = p.y, z = p.z;
        return *this;
    }

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
    // u��������-1, v��������-1, u����, v����
    int maxU, maxV, uOrder, vOrder;
    vector<vector<double> > weight; // Ȩ��
    vector<vector<Point> > pnts;
    vector<vector<double> > T_u; // u �ڵ�ʸ��
    vector<vector<double> > T_v; // v �ڵ�ʸ��
};

NURBS_Surface face1, face2, face3, face4;


void GetKnotVector(NURBS_Surface& face, vector<double>& T, int nCount, int num,int order, bool bU)//������-�ֵ��㷨��ȡ�ڵ�ʸ������
{
    T.clear();
    for (int i = 0; i <= order + num + 1; ++i) {
        if (i <= order) T.push_back(0.0);
        else if (i >= num + 1 && i <= num + order + 1) T.push_back(1.0);
        else if (i >= order + 1 && i <= num) {
            // ����num-order���ڽڵ�
            double sum = 0.0;
            for(int j = order + 1; j <= i; ++j) { 
                double numerator = 0.0;//�������
                for (int loop = j - order; loop <= j - 1; ++loop) {
                    if(bU)//ѡ�����ڵ�ʸ��U���Ǽ���ڵ�ʸ��V
                        numerator += (face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)*(face.pnts[nCount][loop].x-face.pnts[nCount][loop-1].x)+(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y)*(face.pnts[nCount][loop].y-face.pnts[nCount][loop-1].y);
                    else
                        numerator += (face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)*(face.pnts[loop][nCount].x-face.pnts[loop-1][nCount].x)+(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y)*(face.pnts[loop][nCount].y-face.pnts[loop-1][nCount].y);
                }
                double denominator = 0.0;//�����ĸ
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

double BasisFunctionValue(double t, int i, int order, const vector<double>& T) //����B����������
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
			double coffcient1, coffcient2;//͹���ϵ��1��͹���ϵ��2
			double denominator = 0.0;//��ĸ
			denominator = T[i + order] - T[i];//���ƹ�ʽ��һ���ĸ
			if(denominator == 0.0)//Լ��0/0
				coffcient1 = 0.0;
			else
				coffcient1=(t - T[i])/denominator;
			denominator=T[i + order + 1] - T[i + 1]; //���ƹ�ʽ�ڶ����ĸ
			if(0.0 == denominator)//Լ��0/0
				coffcient2 = 0.0;
			else
				coffcient2 = (T[i + order + 1] - t) / denominator;
			value1 = coffcient1 * BasisFunctionValue(t, i, order - 1, T);//���ƹ�ʽ��һ���ֵ
			value2 = coffcient2 * BasisFunctionValue(t, i + 1, order - 1, T);//���ƹ�ʽ�ڶ����ֵ
			value = value1 + value2;//��������ֵ
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

// Ԥ���� ����uv����
void preprocess(NURBS_Surface& face, vector<SuperPoint>& spnts, double step) {
    for (double fv = 0.0; fv <= 1.0; fv += step) {
        for (double fu = 0.0; fu <= 1.0; fu += step) {
            Point pnt = getPntByUV(face, fv, fu);
            spnts.emplace_back(SuperPoint(pnt, fu, fv));
        }
    }
}

double getNearestSuperPoint(vector<SuperPoint>& spnts, const Point& pnt, SuperPoint& nearest) {
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
    handleInput("static/face-1.txt", face1);
    handleInput("static/face-2.txt", face2);
    handleInput("static/face-3.txt", face3);
    handleInput("static/face-4.txt", face4);

    printf("\n����ļ�ΪͬĿ¼�µ�table.csv���������ͬ���Ƶ��ļ����Ḳ�����ݣ�\n");
    printf("\n�������stepΪ������face1�н�uv�ֱ���[0.0, 1.0]��������㣬���Ըõ�Ϊ��׼����face2����ͬ���������������\n�� ����Ϊ0.1��uv����ֱ����11�Σ���121�����");
    printf("\n������ϵͳ��������ʵ�ʵļ��������ƫ����粽��Ϊ0.05ʱ��uv����Ӧ�ֱ����21�Σ���ʵ�ʿ���ֻ��20��\n");
    printf("\n[���벽��step] ");
    double step = 0.01;
    scanf("%lf", &step);
    cout << endl;
    const int times = floor(1.0 / step) + 1;

    ofstream ofile;
    ofile.open("table.csv", ios::out);
    ofile << "face1 uv, face1 xyz, face2 uv, face2 xyz, face2 L, face3 uv, face3 xyz, face3 L, face4 uv, face4 xyz, face4 L, min face, min L" << endl;

    clock_t stime, etime;
    stime = clock();
    
    // Ԥ�ȼ�¼face2, face3, face4�еĵ���
    vector<SuperPoint> face2pnts;
    preprocess(face2, face2pnts, step);
    vector<SuperPoint> face3pnts;
    preprocess(face3, face3pnts, step);
    vector<SuperPoint> face4pnts;
    preprocess(face4, face4pnts, step);

    const int savepoint = 500; // ����500�α���һ������ �����ڴ����

    map<string, int> minFaceCount {
        {"face2", 0},
        {"face3", 0},
        {"face4", 0}
    };
    for (double f1v = 0.0; f1v <= 1.0; f1v += step) {
        for (double f1u = 0.0; f1u <= 1.0; f1u += step) {
            int count = (f1v / step) * times + (f1u / step) + 1;
            if (count % 500 == 0) {
                // ��������
                ofile.close();
                ofile.open("table.csv", ios::app);
            }
            printf("\r[processed] %.3lf%%", 100.0 * count / (times * times));
            // ��������1�еĵ�
            Point pivot = getPntByUV(face1, f1v, f1u);
            SuperPoint f2nearest;
            double f2minDistance = getNearestSuperPoint(face2pnts, pivot, f2nearest);
            SuperPoint f3nearest;
            double f3minDistance = getNearestSuperPoint(face3pnts, pivot, f3nearest);
            SuperPoint f4nearest;
            double f4minDistance = getNearestSuperPoint(face4pnts, pivot, f4nearest);
            // д���ļ�
            double minDistance = min(f2minDistance, min(f3minDistance, f4minDistance));
            string minFace = "face2";
            if ((f3minDistance + EPS) < f2minDistance) {
                minFace = "face3";
                if ((f4minDistance + EPS) < f3minDistance) {
                    minFace = "face4";
                }
                else {
                    cout << f2minDistance << " " << f3minDistance << endl;
                }
            } else if ((f4minDistance + EPS) < f2minDistance) {
                minFace = "face4";
            }
            minFaceCount[minFace] ++;

            ofile << "\"" << f1u << "," << f1v << "\"," \
                  << "\"" << pivot.x << "," << pivot.y << "," << pivot.z << "\"," \
                  << "\"" << f2nearest.u << "," << f2nearest.v << "\"," \
                  << "\"" << f2nearest.pnt.x << "," << f2nearest.pnt.y << "," << f2nearest.pnt.z << "\"," \
                  << f2minDistance << "," \
                  << "\"" << f3nearest.u << "," << f3nearest.v << "\"," \
                  << "\"" << f3nearest.pnt.x << "," << f3nearest.pnt.y << "," << f3nearest.pnt.z << "\"," \
                  << f3minDistance << "," \
                  << "\"" << f4nearest.u << "," << f4nearest.v << "\"," \
                  << "\"" << f4nearest.pnt.x << "," << f4nearest.pnt.y << "," << f4nearest.pnt.z << "\"," \
                  << f4minDistance << "," \
                  << "\"" << minFace << "\"," \
                  << minDistance << endl;
        }
    }
    etime = clock();
    double total_sec = double(etime - stime) / CLOCKS_PER_SEC;
    int cost_min = floor(total_sec / 60);
    double cost_sec = total_sec - cost_min * 60;
    cout << endl << endl << "[��ʱ] " << cost_min << "min " << cost_sec << "s" << endl;

    cout << "\n��С������ͳ��" << endl;

    for (auto& iter : minFaceCount) {
        if (iter.second == 0) continue;
        cout << iter.first << ": " << iter.second << "��" << endl;
    }
    cout << endl;

    ofile.close();

    system("pause");
    return 0;
}