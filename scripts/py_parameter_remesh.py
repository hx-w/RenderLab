# -*- coding: utf-8 -*-

import trimesh
from trimesh import Trimesh
import numpy as np
import numpy_indexed as npi

from typing import List, Tuple


'''
input mesh, find its boundary vertices (open mesh)
@return (
    boundary vertices (np.array), first-last is same
    length of boundary (float)
)
'''
def get_mesh_boundary_cmd(vertices: np.array, faces: np.array) -> Tuple[np.array, float]:
    vertices = vertices.reshape((-1, 3))
    faces = faces.reshape((-1, 3))
    msh = Trimesh(vertices=vertices, faces=faces)
    return _get_mesh_boundary(msh)


def _get_mesh_boundary(msh: Trimesh) -> Tuple[np.array, float]:
    bnd_edges = npi.difference(msh.edges_unique, msh.face_adjacency_edges)

    bnd_verts = np.array([*bnd_edges[0]])
    bnd_edges = np.delete(bnd_edges, [0], axis=0)

    length_sum = 0.0
    # reorder
    success = True
    while success:
        success = False
        last = bnd_verts[-1]
        for idx, edge in enumerate(bnd_edges):
            if last == edge[0]:
                success = True
                last = edge[1]
            elif last == edge[1]:
                success = True
                last = edge[0]
            if success:
                length_sum += np.linalg.norm(msh.vertices[last] - msh.vertices[bnd_verts[-1]])
                bnd_verts = np.append(bnd_verts, last)
                bnd_edges = np.delete(bnd_edges, [idx], axis=0)
                break

    return bnd_verts, length_sum


if __name__ == '__main__':
    msh = trimesh.load("testcases/default/face1.obj")
    print(_get_mesh_boundary(msh))