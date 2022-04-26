#include <ctime>
#include <map>
#include "utils.hpp"
using namespace std;

NURBS_Surface face1, face2, face3, face4;

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

    printf("\n[step] ");
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

    const int savepoint = 500;

    map<string, int> minFaceCount {
        {"face2", 0},
        {"face3", 0},
        {"face4", 0}
    };
    for (double f1v = 0.0; f1v <= 1.0; f1v += step) {
        for (double f1u = 0.0; f1u <= 1.0; f1u += step) {
            int count = (f1v / step) * times + (f1u / step) + 1;
            if (count % 500 == 0) {
                ofile.close();
                ofile.open("table.csv", ios::app);
            }
            printf("\r[processed] %.3lf%%", 100.0 * count / (times * times));
            // foreach face
            Point pivot = getPntByUV(face1, f1v, f1u);
            SuperPoint f2nearest;
            double f2minDistance = getNearestSuperPoint(face2pnts, pivot, f2nearest);
            SuperPoint f3nearest;
            double f3minDistance = getNearestSuperPoint(face3pnts, pivot, f3nearest);
            SuperPoint f4nearest;
            double f4minDistance = getNearestSuperPoint(face4pnts, pivot, f4nearest);
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
    cout << endl << endl << "[time cost] " << cost_min << "min " << cost_sec << "s" << endl;

    cout << "\nsummary" << endl;

    for (auto& iter : minFaceCount) {
        if (iter.second == 0) continue;
        cout << iter.first << ": " << iter.second << "次" << endl;
    }
    cout << endl;

    ofile.close();

#ifdef __WIN32__
    system("pause");
#endif
    return 0;
}