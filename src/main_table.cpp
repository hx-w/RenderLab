#include <ctime>
#include <map>
#include "utils.hpp"
using namespace std;

NURBS_Surface face1, face2, face3, face4;

void preprocess(NURBS_Surface& face, vector<SuperPoint>& spnts, int max_iter) {
    for (int fv = 0; fv < max_iter; ++fv) {
        for (int fu = 0; fu < max_iter; ++fu) {
            Point pnt = getPntByUV(face, double(fv) / max_iter, double(fu) / max_iter);
            spnts.emplace_back(SuperPoint(pnt, fu, fv));
        }
    }
}

double getNearestSuperPoint(const vector<SuperPoint>& spnts, const Point& pnt, SuperPoint& nearest) {
    nearest = spnts[0];
    double minDistance = getDistance(pnt, nearest);
    for (auto& spnt : spnts) {
        double distance = getDistance(pnt, spnt);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = spnt;
        }
    }
    return minDistance;
}

void genEdgeSetNorm(EdgePointSet& edge_set, NURBS_Surface& face) {
    int len = edge_set.pnts.size();
    for (int i = 0; i < len; ++i) {
        if (edge_set.is_edge(edge_set.pnts.at(i))) {
            edge_set.true_edge_idx.push_back(i);
            auto pl = getPntByUV(face, double(edge_set.pnts.at(i).v - 1) / edge_set.max_iter, double(edge_set.pnts.at(i).u) / edge_set.max_iter);
            auto pr = getPntByUV(face, double(edge_set.pnts.at(i).v + 1) / edge_set.max_iter, double(edge_set.pnts.at(i).u) / edge_set.max_iter);
            auto pt = getPntByUV(face, double(edge_set.pnts.at(i).v) / edge_set.max_iter, double(edge_set.pnts.at(i).u + 1) / edge_set.max_iter);
            auto pd = getPntByUV(face, double(edge_set.pnts.at(i).v) / edge_set.max_iter, double(edge_set.pnts.at(i).u - 1) / edge_set.max_iter);
            auto nlt = calcNormal(pl, pt, edge_set.pnts.at(1));
            auto nrt = calcNormal(pr, pt, edge_set.pnts.at(1));
            auto nld = calcNormal(pl, pd, edge_set.pnts.at(1));
            auto nrd = calcNormal(pr, pd, edge_set.pnts.at(1));
            auto nava = (nlt + nrt + nld + nrd) / 4;
            edge_set.norms.push_back(nava);
            edge_set.edge_u_idx[edge_set.pnts.at(i).u].push_back(i);
            edge_set.edge_v_idx[edge_set.pnts.at(i).v].push_back(i);
        }
    }
}

void getEdgeSet(EdgePointSet& edge_set, int max_iter, const vector<SuperPoint>& face2pnts, const vector<SuperPoint>& face3pnts, NURBS_Surface& face) {
    for (int f1v = 0; f1v < max_iter; ++f1v) {
        for (int f1u = 0; f1u < max_iter; ++f1u) {
            Point pivot = getPntByUV(face1, double(f1v) / max_iter, double(f1u) / max_iter);
            SuperPoint f2nearest;
            double f2minDistance = getNearestSuperPoint(face2pnts, pivot, f2nearest);
            SuperPoint f3nearest;
            double f3minDistance = getNearestSuperPoint(face3pnts, pivot, f3nearest);
            if (f3minDistance <= f2minDistance) {
                edge_set.add(SuperPoint(pivot, f1u, f1v));
            }
        }
    }
    genEdgeSetNorm(edge_set, face);
    ofstream ofile;
    ofile.open("edge.csv", ios::out);
    ofile << "x,y,z" << endl;
    for (auto& idx : edge_set.true_edge_idx) {
        ofile << edge_set.pnts[idx].x << "," << edge_set.pnts[idx].y << "," << edge_set.pnts[idx].z << endl;
    }
    ofile.close();
}

#define EPS 1e-8

int main() {
    handleInput("static/face-1.txt", face1);
    handleInput("static/face-2.txt", face2);
    handleInput("static/face-3.txt", face3);
    handleInput("static/face-4.txt", face4);

    printf("\n[max_iter] ");
    int max_iter = 100;
    scanf("%d", &max_iter);
    cout << endl;

    ofstream ofile;
    ofile.open("table.csv", ios::out);
    ofile << "face1 uv, face1 xyz, face2 uv, face2 xyz, face2 L, face3 uv, face3 xyz, face3 L, face4 uv, face4 xyz, face4 L, min face, min L" << endl;

    clock_t stime, etime;
    stime = clock();
    
    vector<SuperPoint> face2pnts;
    preprocess(face2, face2pnts, max_iter);
    vector<SuperPoint> face3pnts;
    preprocess(face3, face3pnts, max_iter);
    vector<SuperPoint> face4pnts;
    preprocess(face4, face4pnts, max_iter);

    const int savepoint = 500;

    EdgePointSet edge_set(max_iter); // for face1
    getEdgeSet(edge_set, max_iter, face2pnts, face3pnts, face1);

    map<string, int> minFaceCount {
        {"face2", 0},
        {"face3", 0},
        {"face4", 0}
    };

    for (int f1v = 0; f1v < max_iter; ++f1v) {
        for (int f1u = 0; f1u < max_iter; ++f1u) {
            int count = f1v * max_iter + f1u + 1;
            if (count % 500 == 0) {
                ofile.close();
                ofile.open("table.csv", ios::app);
            }
            printf("\r[processed] %.3lf%%", 100.0 * count / (max_iter * max_iter));
            // foreach face
            Point pivot = getPntByUV(face1, double(f1v) / max_iter, double(f1u) / max_iter);
            SuperPoint f2nearest;
            double f2minDistance = getNearestSuperPoint(face2pnts, pivot, f2nearest);
            SuperPoint f3nearest;
            double f3minDistance = getNearestSuperPoint(face3pnts, pivot, f3nearest);
            SuperPoint f4nearest;
            double f4minDistance = getNearestSuperPoint(face4pnts, pivot, f4nearest);
            double minDistance = min(f2minDistance, min(f3minDistance, f4minDistance));
            string minFace = "face2";
            // 判断pivot是否在edge_set中
            if (edge_set.is_in_set(f1u, f1v)) {
                minFace = "face4";
                Point norm;
                bool is_edge = false;
                for (int idx = 0; idx < edge_set.true_edge_idx.size(); ++idx) {
                    if (edge_set.pnts[edge_set.true_edge_idx[idx]].u == f1u && edge_set.pnts[edge_set.true_edge_idx[idx]].v == f1v) {
                        norm = edge_set.norms[idx];
                        is_edge = true;
                        break;
                    }
                }
                if (!is_edge) {
                    // 代选法线列表
                    vector<int> waiting;
                    for (auto idx : edge_set.edge_u_idx[f1u]) {
                        waiting.push_back(idx);
                    }
                    for (auto idx : edge_set.edge_v_idx[f1v]) {
                        waiting.push_back(idx);
                    }
                    cout << waiting.size() << endl;
                }
            }


            minFaceCount[minFace] ++;

            ofile << "\"" << double(f1u) / max_iter << "," << double(f1v) / max_iter << "\"," \
                  << "\"" << pivot.x << "," << pivot.y << "," << pivot.z << "\"," \
                  << "\"" << f2nearest.u << "," << f2nearest.v << "\"," \
                  << "\"" << f2nearest.x << "," << f2nearest.y << "," << f2nearest.z << "\"," \
                  << f2minDistance << "," \
                  << "\"" << f3nearest.u << "," << f3nearest.v << "\"," \
                  << "\"" << f3nearest.x << "," << f3nearest.y << "," << f3nearest.z << "\"," \
                  << f3minDistance << "," \
                  << "\"" << f4nearest.u << "," << f4nearest.v << "\"," \
                  << "\"" << f4nearest.x << "," << f4nearest.y << "," << f4nearest.z << "\"," \
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