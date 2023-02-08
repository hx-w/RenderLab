# -*- coding: utf-8 -*-

import trimesh
from trimesh import Trimesh
import numpy as np
import numpy_indexed as npi
from functools import reduce
from scipy.sparse import csc_matrix
from scipy.sparse.linalg import spsolve
from tqdm import tqdm

from copy import deepcopy
from typing import List, Tuple


'''
since the low efficiency of indicing Trimesh.edges_unique_length
need to return edge length in mesh
'''
def mesh_vert_dist(msh: Trimesh, vidx1: int, vidx2: int) -> float:
    return np.linalg.norm(msh.vertices[vidx1] - msh.vertices[vidx2])

'''
return angle of two vectors conformed by three vertices
'''
def mesh_vert_angle(msh: Trimesh, mid: int, start: int, end: int) -> float:
    vec1 = msh.vertices[start] - msh.vertices[mid]
    vec2 = msh.vertices[end] - msh.vertices[mid]
    return np.arccos(vec1.dot(vec2) / (np.linalg.norm(vec1) * np.linalg.norm(vec2)))


def math_cot(angle: float) -> float:
    return np.cos(angle) / np.sin(angle)


'''
return cross product of vec: vec1 - vec2 and vec3 - vec2
'''
def mesh_vec_cross(vec1: tuple, vec2: tuple, vec3: tuple) -> float:
    vt1 = (vec1[0] - vec2[0], vec1[1] - vec2[1])
    vt2 = (vec3[0] - vec2[0], vec3[1] - vec2[1])
    return vt1[0] * vt2[1] - vt2[0] * vt1[1]

'''
return triangle area with vertices: vec1, vec2, vec3
'''
def mesh_trias_area(vec1: np.array, vec2: np.array, vec3: np.array) -> float:
    _a = np.linalg.norm(vec2 - vec1)
    _b = np.linalg.norm(vec3 - vec1)
    _c = np.linalg.norm(vec3 - vec2)
    _s = (_a + _b + _c) / 2
    return (_s * (_s - _a) * (_s - _b) * (_s - _c)) ** 0.5


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
                bnd_verts = np.append(bnd_verts, last)
                bnd_edges = np.delete(bnd_edges, [idx], axis=0)
                break

    bnd_verts = np.append(bnd_verts, bnd_verts[0])
    length_sum = sum([mesh_vert_dist(msh, *bnd_verts[i:i+2]) for i in range(len(bnd_verts) - 1)])
    bnd_verts = bnd_verts[:-2]
    return bnd_verts, length_sum

'''
*** initial weights ***
'''
def _initialize_weights(msh: Trimesh, inn_verts: list, bnd_verts: list) -> csc_matrix:
    # sub function
    def weights_for_edge(edge: list) -> float:
        adj_list_s = msh.vertex_neighbors[edge[0]]
        adj_list_b = msh.vertex_neighbors[edge[1]]
        adj_vts = npi.intersection(adj_list_s, adj_list_b)
        # assert len(adj_vts) == 2, 'not a manifold'
        # compute cotangent weight of edge
        ang1 = mesh_vert_angle(msh, adj_vts[0], *edge)
        ang2 = mesh_vert_angle(msh, adj_vts[1], *edge)
        _w = (math_cot(ang1) + math_cot(ang2)) / 2
        return -_w

    # sparse matrix index
    sp_row = np.array([], dtype=int)
    sp_col = np.array([], dtype=int)
    sp_data = np.array([], dtype=float)
    mtx_diag = np.zeros(len(msh.vertices))
    # generate
    _weights = list(map(weights_for_edge, tqdm(msh.face_adjacency_edges, desc='generating weights')))
    # update diag
    for idx, edge in enumerate(msh.face_adjacency_edges):
        mtx_diag[edge[0]] += -_weights[idx]
        mtx_diag[edge[1]] += -_weights[idx]
    
    # transpose indices
    _indices = msh.face_adjacency_edges.T
    sp_row = np.hstack([sp_row, _indices[0], _indices[1]])
    sp_col = np.hstack([sp_col, _indices[1], _indices[0]])
    sp_data = np.hstack([sp_data, _weights, _weights])

    # handle diag sparse index
    # all vertices in msh with order {INNER, BOUND}
    sp_diag_index = np.append(inn_verts, bnd_verts)
    sp_row = np.hstack([sp_row, sp_diag_index])
    sp_col = np.hstack([sp_col, sp_diag_index])
    sp_diag_data = [mtx_diag[v] for v in sp_diag_index]
    sp_data = np.hstack([sp_data, sp_diag_data])

    sp_weights = csc_matrix((sp_data, (sp_row, sp_col)), dtype=float)
    return sp_weights


def _mapping_fixed_boundary(msh: Trimesh, bnds: np.array, sum_len: float, scale: float = 2.0) -> Tuple[np.array]:
    bnds = np.append(bnds, bnds[0])
    f_B = np.empty((0, 2))

    last_v = bnds[0]
    bnds = bnds[1:]
    accumed = 0.
    _first = 3
    for bnd in bnds: # last_v to bnd
        old_ratio = accumed / sum_len
        accumed += np.linalg.norm(msh.vertices[last_v] - msh.vertices[bnd])
        new_ratio = accumed / sum_len
        flag = -reduce(
            lambda x, y: x * (
                1 if ((y - old_ratio) * (y - new_ratio)) > 0 
                else -y
            ),
            [0.25, 0.5, 0.75],
            1
        )
        if flag > 0:
            new_ratio = flag
        vpos = (0., 0.)
        if new_ratio < 0.25:
            vpos = (-(scale / 2) + scale * (new_ratio / 0.25), scale / 2)
        elif new_ratio < 0.5:
            vpos = (scale / 2,  (scale / 2) - scale * ((new_ratio - 0.25) / 0.25))
        elif new_ratio < 0.75:
            vpos = ((scale / 2) - scale * ((new_ratio - 0.5) / 0.25), -scale / 2)
        else:
            vpos = (-scale / 2, -(scale / 2) + scale * ((new_ratio - 0.75) / 0.25))
        if _first > 0:
            _first -= 1
        f_B = np.append(f_B, [vpos], axis=0)
        last_v = bnd
    return f_B, bnds


'''
split sp_weights with sp_weights_II and sp_weights_IB
and solve equation:
    sp_weights_II * f_I = -sp_weights_IB * f_B
'''
def solve_equation(sp_weights: csc_matrix, f_B: list, inn_verts: list, bnd_verts: list) -> list:
    _mid = sp_weights[inn_verts, ...]
    sp_weights_II = _mid[..., inn_verts]
    sp_weights_IB = _mid[..., bnd_verts]

    assert sp_weights_IB.shape[1] == len(f_B), f'L_IB({sp_weights.shape[1]}) * f_B({len(f_B)}) illegal'

    f_I = spsolve(sp_weights_II, -sp_weights_IB * f_B)
    return f_I


'''
build param mesh by inverse mapping
assume Z=0 in param mesh
'''
def build_param_mesh(msh: Trimesh, inn_verts: list, bnd_verts: list, f_I: list, f_B: list) -> Trimesh:
    len_inn, len_bnd = len(inn_verts), len(bnd_verts)
    param_bnd_verts = [v + len_inn for v in range(len_bnd)]
    inv_mapping = dict(zip(bnd_verts, param_bnd_verts))
    param_inn_verts = [v for v in range(len_inn)]
    inv_mapping.update(zip(inn_verts, param_inn_verts))
    param_tot = np.append(f_I, f_B, axis=0)
    # param_tot add new column Z=0
    param_tot = np.hstack([param_tot, np.zeros((len(param_tot), 1))])

    param_mesh = Trimesh(
        vertices=[param_tot[inv_mapping[i]] for i in range(len_inn + len_bnd)],
        faces=deepcopy(msh.faces)
    )
    print('check: ', param_mesh.vertices[bnd_verts[0]], 'for', bnd_verts[0])
    param_mesh.remove_degenerate_faces()
    param_mesh.remove_duplicate_faces()
    param_mesh.remove_infinite_values()
    return param_mesh


def build_str_mesh_custom(uns_mesh: Trimesh, param_mesh: Trimesh, sample_pnts: list, scale: float=2.) -> Trimesh:
    square_nums = len(sample_pnts)
    assert square_nums > 4, 'sample points illegal'
    sample_nums = int(square_nums ** 0.5)
    # flatten numpy elements to list will accelerate in cycle
    flt_faces = param_mesh.faces.tolist()
    flt_area_faces = param_mesh.area_faces.tolist()
    str_mesh = Trimesh()

    sample_trias = param_mesh.nearest.on_surface(sample_pnts)
    sample_trias = sample_trias[2].tolist()
    spot_trias = list(map(lambda tri: flt_faces[tri], sample_trias))
    vijk_areas = [
        [
            mesh_trias_area(
                sample_pnts[idx],
                param_mesh.vertices[spot_trias[idx][1]],
                param_mesh.vertices[spot_trias[idx][2]]
            ),
            mesh_trias_area(
                param_mesh.vertices[spot_trias[idx][0]],
                sample_pnts[idx],
                param_mesh.vertices[spot_trias[idx][2]]
            ),
            mesh_trias_area(
                param_mesh.vertices[spot_trias[idx][0]],
                param_mesh.vertices[spot_trias[idx][1]],
                sample_pnts[idx],
            )
        ]
        for idx in range(square_nums)
    ]

    str_pnts = [
        (
            vijk_areas[idx][0] * uns_mesh.vertices[spot_trias[idx][0]] +
            vijk_areas[idx][1] * uns_mesh.vertices[spot_trias[idx][1]] +
            vijk_areas[idx][2] * uns_mesh.vertices[spot_trias[idx][2]]
        ) / flt_area_faces[sample_trias[idx]]
        for idx in range(square_nums)
    ]

    half_trias1 = [
        [ir * sample_nums + ic, ir * sample_nums + ic - sample_nums, ir * sample_nums + ic - 1]
        for ir in range(1, sample_nums) for ic in range(1, sample_nums)
    ]
    half_trias2 = [
        [ir * sample_nums + ic - 1, ir * sample_nums + ic - sample_nums, ir * sample_nums + ic - sample_nums - 1]
        for ir in range(1, sample_nums) for ic in range(1, sample_nums)
    ]

    str_mesh.vertices = str_pnts
    str_mesh.faces = np.vstack([half_trias1, half_trias2])

    str_mesh.remove_infinite_values()
    str_mesh.remove_degenerate_faces()
    str_mesh.remove_unreferenced_vertices()
    str_mesh.fill_holes()
    str_mesh.fix_normals()

    return str_mesh

'''
build str mesh by sample vertices on param mesh
'''
def build_str_mesh(uns_mesh: Trimesh, param_mesh: Trimesh, U: int, V: int, scale: float=2.) -> Trimesh:
    assert U > 2, 'sample_nums too small'
    # flatten numpy elements to list will accelerate in cycle
    sample_pnts = []
    for ic in range(V):
        for ir in range(U):
            sample_pnts.append([-scale * ir / (U - 1) + scale / 2, -scale * ic / (V - 1) + scale / 2,  0.])

    return build_str_mesh_custom(uns_mesh, param_mesh, sample_pnts, scale)


def _parameter_remesh(msh: Trimesh, U: int, V: int, pivot: int, order: bool = True) -> Tuple[np.array]:
    bnd_verts, length_sum = _get_mesh_boundary(msh)
    inn_verts = npi.difference(msh.face_adjacency_edges.flatten(), bnd_verts)
    if not order: # reverse
        bnd_verts = bnd_verts[::-1]
    bnd_verts = np.roll(bnd_verts, -np.where(bnd_verts == pivot)[0][0])

    f_B, bnd_verts = _mapping_fixed_boundary(msh, bnd_verts, length_sum)

    sp_weights = _initialize_weights(msh, inn_verts, bnd_verts)

    f_I = solve_equation(sp_weights, f_B, inn_verts, bnd_verts)
    param_msh = build_param_mesh(msh, inn_verts, bnd_verts, f_I, f_B)
    str_msh = build_str_mesh(msh, param_msh, U, V)

    return str_msh, param_msh


'''
@return (remesh_vertices, remesh_faces, parameter_vertices, parameter_faces)
'''
def parameter_remesh_cmd(vertices: np.array, faces: np.array, U: int, V: int, pivot: int, order: bool = True) -> Tuple[np.array]:
    vertices = vertices.reshape((-1, 3))
    faces = faces.reshape((-1, 3))
    msh = Trimesh(vertices=vertices, faces=faces)
    re_msh, prm_msh = _parameter_remesh(msh, U, V, pivot, order)
    return re_msh.vertices, re_msh.faces, prm_msh.vertices, prm_msh.faces


if __name__ == '__main__':
    msh = trimesh.load("testcases/default/face1.obj")
    pivot = 919
    r_verts, r_faces, p_verts, p_faces = parameter_remesh_cmd(msh.vertices, msh.faces, 10, 100, pivot, False)
    print(r_faces)
