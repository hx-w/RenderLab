#include "parameterization.h"
#include <iostream>
#include <map>
using namespace std;

namespace RenderSpace {
    Parameterization::Parameterization(MeshDrawable* ori, MeshDrawable* tar):
        m_ori(ori), m_tar(tar) {}

    Parameterization::~Parameterization() {}

    void Parameterization::parameterize() {
        vector<int> vidx_bound;
        vector<int> vidx_inner;
        _remark_vertices(vidx_bound, vidx_inner);
    }

    void Parameterization::_remark_vertices(std::vector<int>& vidx_bound, std::vector<int>& vidx_inner) {
        // 构造Edge集合，判断只与一个三角形相邻的边
    }
}
