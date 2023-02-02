# -*- coding: utf-8 -*-
from typing import Tuple
import numpy as np

'''
通过形值点points, 反算NURBS曲线的控制点, 权重, 以及knot向量
曲线为开曲线

knot计算方法: 积累弦长参数化法
边界条件: 切矢条件
曲线阶数: 3

仅仅使用numpy和matplotlib, 不使用任何第三方库
'''

k = 3

def compute_knots(points: np.ndarray, k: int = 3) -> np.ndarray:
    n = points.shape[0]
    chord = np.linalg.norm(points[1:] - points[:-1], axis=1)
    sum_chord = np.sum(chord)

    knots = np.zeros(n + k + 3)
    knots[-k - 1:] = 1
    knots[k: n + k - 1] = np.cumsum(chord) / sum_chord
    knots = np.roll(knots, 1)
    knots[0] = 0.0
    return knots


def compute_control_points(points: np.ndarray, knots: np.ndarray, k: int = 3) -> np.ndarray:
    n = points.shape[0]
    knots = compute_knots(points, k)

    dpt1 = np.array([0, 0, 1])
    dptn = np.array([-1, 0, 0])

    dknots = np.diff(knots)
    dknots = np.append(dknots, 0.0)
    dknots[dknots < 1e-6] = 0

    A = np.zeros((n, n))
    E = np.zeros((n, 3))

    A[0, 0] = 1
    A[-1, -1] = 1
    E[0, :] = points[0] + (dknots[3] / 3) * dpt1   # ???
    E[-1, :] = points[-1] - (dknots[-4] / 3) * dptn

    for i in range(1, n - 1):
        A[i, i - 1] = (dknots[i + 3] ** 2) / (dknots[i + 1] + dknots[i + 2] + dknots[i + 3])
        A[i, i] = dknots[i + 3] * (dknots[i + 1] + dknots[i + 2]) / (dknots[i + 1] + dknots[i + 2] + dknots[i + 3]) + \
                dknots[i + 2] * (dknots[i + 3] + dknots[i + 4]) / (dknots[i + 2] + dknots[i + 3] + dknots[i + 4])
        A[i, i + 1] = (dknots[i + 2] ** 2) / (dknots[i + 2] + dknots[i + 3] + dknots[i + 4])
        E[i, :] = (dknots[i + 2] + dknots[i + 3]) * points[i]

    D = np.linalg.solve(A, E)
    D = np.vstack((points[0], D, points[-1]))
    np.set_printoptions(precision=6, suppress=True)
    return D


def nurbs_curve(knots: np.ndarray, D: np.ndarray, k: int = 3, dt: float = 0.01) -> np.ndarray:
    def basis_function(i: int, k: int, knot: float, knots: np.ndarray) -> float:
        if k == 1:
            if knots[i] <= knot < knots[i + 1]:
                return 1
            else:
                return 0
        else:
            if knots[i + k - 1] == knots[i]:
                c1 = 0
            else:
                c1 = (knot - knots[i]) / (knots[i + k - 1] - knots[i]) * basis_function(i, k - 1, knot, knots)
            if knots[i + k] == knots[i + 1]:
                c2 = 0
            else:
                c2 = (knots[i + k] - knot) / (knots[i + k] - knots[i + 1]) * basis_function(i + 1, k - 1, knot, knots)
            return c1 + c2

    n = D.shape[0]
    u = np.arange(knots[k], knots[-k], dt)
    curve = np.zeros((u.shape[0], D.shape[1]))
    for i in range(u.shape[0]):
        for j in range(n):
            curve[i, :] += basis_function(j, k, u[i], knots) * D[j, :]
    return curve


def compute_nurbs_reverse(points: np.ndarray, k: int = 3) -> Tuple[np.ndarray, np.ndarray]:
    points = points.reshape((-1, 3))
    knots = compute_knots(points, k)
    D = compute_control_points(points, knots, k)
    return knots, D


def plot_curve(points: np.ndarray, k: int = 3):
    points = points.reshape((-1, 3))
    from matplotlib import pyplot as plt

    knots, D = compute_nurbs_reverse(points, k)
    curve = nurbs_curve(knots, D, k)
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.plot(curve[:, 0], curve[:, 1], curve[:, 2], 'r')    
    ax.plot(points[:, 0], points[:, 1], points[:, 2], c='b')
    ax.scatter(D[:, 0], D[:, 1], D[:, 2], c='g')
    plt.show()
    return knots, D


if __name__ == '__main__':
    '''
    形值点 (3D)
    '''
    points = np.array([
        [0, 0, 0],
        [3, 11, 1],
        [21, 17, 2],
        [26, 6, 3],
        [32, 1, 10],
        [60, 13, 11],
        [50, 23, 15],
        [70, 28, 20],
        [75, 35, 25]
    ])

    knots, D = plot_curve(points, 3)

    print(points.shape, knots.shape, D.shape)
