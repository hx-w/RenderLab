//! 使用LU分解求解方程组
#pragma once

#include <iostream>
#include <unordered_map>
#include <utility>
#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace glm_ext {
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};

template <typename T>
class SparseMatrix {
public:
    SparseMatrix(int n, int m) : _n(n), _m(m) {}
    ~SparseMatrix() {}

    void set(int i, int j, T v) {
        if (i < 0 || i >= _n || j < 0 || j >= _m) {
            return;
        }
        _mat[std::make_pair(i, j)] = v;
    }

    T get(int i, int j) const {
        if (i < 0 || i >= _n || j < 0 || j >= _m) {
            return static_cast<T>(0);
        }
        if (_mat.find(std::make_pair(i, j)) == _mat.end()) {
            return static_cast<T>(0);
        }
        return _mat.at(std::make_pair(i, j));
    }

#if 1
    void print() {
        for (int i = 0; i < _n; ++i) {
            for (int j = 0; j < _m; ++j) {
                printf("%f ", get(i, j));
            }
            printf("\n");
        }
        printf("\n");
    }
#endif

    int n() const { return _n; }
    int m() const { return _m; }

private:
    int _n;
    int _m;
    std::unordered_map<std::pair<int, int>, T, pair_hash> _mat;
};

template <typename T>
static void Doolitte_decompose(const SparseMatrix<T>& ori, SparseMatrix<T>& L, SparseMatrix<T>& U) {
    const int n = ori.n();
    const int m = ori.m();
    L = SparseMatrix<T>(n, m);
    U = SparseMatrix<T>(n, m);
    // implement LU decomposition(Doolitte)
    for (int k = 0; k < n; ++k) {
        for (int j = k; j < n; ++j) {           // dolittle分解
            U.set(k, j, ori.get(k, j));
            for (int i = 0; i <= k - 1; ++i) {
                auto _v = U.get(k, j);
                U.set(k, j, _v - L.get(k, i) * U.get(i, j));
            }
        }
        L.set(k, k, static_cast<T>(1));
        for (int i = k + 1; i < n; ++i) {
            L.set(i, k, ori.get(i, k));
            for (int j = 0; j <= k - 1; ++j) {
                auto _v = L.get(i, k);
                L.set(i, k, _v - L.get(i, j) * U.get(j, k));
            }
            L.set(i, k, L.get(i, k) / U.get(k, k));
        }
    }
}

template <typename T1, typename T2>
static void solve_equations(const SparseMatrix<T1>& A, const SparseMatrix<T2>& b, SparseMatrix<T2>& x) {
    const int n = A.n();
    const int m = A.m();
    assert(A.n() == b.n());
    SparseMatrix<T1> L(n, m);
    SparseMatrix<T1> U(n, m);
    Doolitte_decompose<T1>(A, L, U);

    // solve Ly = b
    SparseMatrix<T2> y(n, 1);
    for (int i = 0; i < n; ++i) {
        y.set(i, 0, b.get(i, 0));
        for (int j = 0; j < i; ++j) {
            y.set(i, 0, y.get(i, 0) - L.get(i, j) * y.get(j, 0));
        }
    }

    // solve Ux = y
    x = SparseMatrix<T2>(n, 1);
    for (int i = n - 1; i >= 0; --i) {
        x.set(i, 0, y.get(i, 0));
        for (int j = i + 1; j < n; ++j) {
            x.set(i, 0, x.get(i, 0) - U.get(i, j) * x.get(j, 0));
        }
        x.set(i, 0, x.get(i, 0) / U.get(i, i));
    }
}

}
