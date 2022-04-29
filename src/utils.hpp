#pragma once
#include <iostream>
#include <fstream>
#include "nurbs_lib.hpp"

using namespace std;

static void handleInput(const string& filename, NURBS_Surface& face) {
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

static const Point getPntByUV(NURBS_Surface& face, double v, double u) {
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
