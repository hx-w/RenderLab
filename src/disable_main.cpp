#include "utils.hpp"
using namespace std;

#define ESP 1e-6

NURBS_Surface face1, face2, face3, face4;

int main() {
    handleInput("static/face-1.txt", face1);
    handleInput("static/face-2.txt", face2);
    handleInput("static/face-3.txt", face3);
    handleInput("static/face-4.txt", face4);

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
    cout << "[point]: " << f2minSp.x << ", " << f2minSp.y << ", " << f2minSp.z << endl;
    cout << "-----------------" << endl;

    cout << "曲面-3最近点" << endl;
    cout << "[distance]: " << f3minDis << endl;
    cout << "[u, v]: " << f3minSp.u << ", " << f3minSp.v << endl;
    cout << "[point]: " << f3minSp.x << ", " << f3minSp.y << ", " << f3minSp.z << endl;
    cout << "-----------------" << endl;

    cout << "曲面-4最近点" << endl;
    cout << "[distance]: " << f4minDis << endl;
    cout << "[u, v]: " << f4minSp.u << ", " << f4minSp.v << endl;
    cout << "[point]: " << f4minSp.x << ", " << f4minSp.y << ", " << f4minSp.z << endl;
    cout << "-----------------" << endl;

    cout << "↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓" << endl << "最近点" << endl;
    cout << "[distance]: " << minDis << endl;
    cout << "[u, v]: " << minSp.u << ", " << minSp.v << endl;
    cout << "[point]: " << minSp.x << ", " << minSp.y << ", " << minSp.z << endl;
    cout << "[face]: " << minFace << endl << endl;

#ifdef __WIN32__
    system("pause");
#endif
    return 0;
}