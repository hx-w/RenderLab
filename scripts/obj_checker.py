# -*- coding:utf-8 -*-

import sys
from copy import deepcopy
from typing import List, Dict


class Vertex:
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z

    def __repr__(self) -> str:
        return f'Vertex({self.x}, {self.y}, {self.z})'


class Triangle:
    def __init__(self, v1: int, v2: int, v3: int):
        self.vts = [v1, v2, v3]

    def __repr__(self) -> str:
        return f'Tri({self.vts[0]}, {self.vts[1]}, {self.vts[2]})'


class MeshValidator:
    def __init__(self, path: str):
        self.path = path
        self.vertices: List[Vertex] = []
        self.triangles: List[Triangle] = []

    def read_obj(self):
        self.vertices = []
        self.triangles = []
        f = open(self.path, 'r')
        for line in f:
            if line.startswith('v'):
                x, y, z = line.split()[1:]
                self.vertices.append(Vertex(float(x), float(y), float(z)))
            elif line.startswith('f'):
                v1, v2, v3 = line.split()[1:]
                self.triangles.append(
                    Triangle(int(v1) - 1, int(v2) - 1, int(v3) - 1))
        f.close()

    def save_obj(self):
        f = open(self.path[:-4] + '_modified.obj', 'w')
        for v in self.vertices:
            f.write(f'v {v.x} {v.y} {v.z}\n')
        for tri in self.triangles:
            f.write(f'f {tri.vts[0] + 1} {tri.vts[1] + 1} {tri.vts[2] + 1}\n')
        f.close()

    def validate(self):
        # valid vertex
        self.valid_vertex_idx: List[int] = []
        for tri in self.triangles:
            if tri.vts[0] not in self.valid_vertex_idx:
                self.valid_vertex_idx.append(tri.vts[0])
            if tri.vts[1] not in self.valid_vertex_idx:
                self.valid_vertex_idx.append(tri.vts[1])
            if tri.vts[2] not in self.valid_vertex_idx:
                self.valid_vertex_idx.append(tri.vts[2])
        self.valid_vertex_idx.sort()
        vvi_size = len(self.valid_vertex_idx)
        tvi_size = len(self.vertices)
        if vvi_size != tvi_size:
            print(
                '[WARN] total vertex:', tvi_size,
                'valid vertex:', vvi_size
            )
        else:
            print('[INFO] total vertex:', tvi_size)
            return
        self.__trim()

    def __trim(self):
        '''
        删除无效顶点
        '''
        self.invalid_vertex_idx_map: Dict[int, int] = {}
        vtsize = len(self.vertices)
        counter = 1
        for vidx in range(vtsize):
            if vidx in self.valid_vertex_idx:
                continue
            self.invalid_vertex_idx_map[vidx] = counter
            counter += 1

        for tri in self.triangles:
            for i in range(3):
                lastvidx, lastc = list(self.invalid_vertex_idx_map.items())[-1]
                if tri.vts[i] > lastvidx:
                    tri.vts[i] -= lastc
                    continue
                for vidx, c in self.invalid_vertex_idx_map.items():
                    if vidx > tri.vts[i]:
                        tri.vts[i] -= c - 1
                        break
        self.vertices = [self.vertices[i] for i in self.valid_vertex_idx]


if __name__ == '__main__':
    '''
    Usage: python obj_checker.py <path>
    '''
    assert(len(sys.argv) == 2)
    mesher = MeshValidator(sys.argv[1])
    mesher.read_obj()
    mesher.validate()
    mesher.save_obj()
