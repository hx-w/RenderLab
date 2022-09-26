'''
for opened mesh parameterization
'''
from copy import deepcopy
from functools import reduce
import numpy_indexed as npi
from scipy.sparse import csc_matrix
from scipy.sparse.linalg import spsolve
from tqdm import tqdm
from utils import *

'''
return (
    inn_verts: list,
    bnd_verts: list,
    bnd_length: float
)
'''
def split_bnd_inn(msh: Trimesh) -> tuple:
    bnd_edges = npi.difference(msh.edges_unique, msh.face_adjacency_edges)

    bnd_verts = np.array([*bnd_edges[0]])
    bnd_edges = np.delete(bnd_edges, [0], axis=0)

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

    inn_verts = npi.difference(msh.face_adjacency_edges.flatten(), bnd_verts)
    return (inn_verts, bnd_verts)

'''
parameterize mesh boundary to square (with pivot limits)
return f_B: list
'''
def mapping_boundary(msh: Trimesh, bnd_verts: list, pivots: list, scale: float=2.) -> list:
    # find pivots[0] in bnd_verts, and shift it to the first with rotate
    bnd_verts = np.roll(bnd_verts, -np.where(bnd_verts == pivots[0])[0][0])
    # split bnd_verts into 4 parts
    splitted = np.split(bnd_verts, [
        np.where(bnd_verts == pivots[1])[0][0],
        np.where(bnd_verts == pivots[2])[0][0],
        np.where(bnd_verts == pivots[3])[0][0],
    ])
    # empty ndarray with shape (n, 2)
    f_B = np.empty((0, 2))
    for i in range(4):
        sub_f_B = mapping_1_boundary(msh, splitted[i], splitted[(i + 1) % 4][0], scale)
        f_B = np.append(f_B, __rotate_90(sub_f_B, i), axis=0)
    
    return f_B

def mapping_1_boundary(msh: Trimesh, sub_bnds: list, last_idx: int, scale: float) -> list:
    '''
    将边缘分为四部分，映射其中一部分，都映射到(-scale/2, scale/2) 和 (scale/2, scale/2)之间
    之后再旋转即可拼凑出四部分映射
    '''
    sub_bnds = np.append(sub_bnds, last_idx)
    bnd_size = len(sub_bnds)
    lengths = [mesh_vert_dist(msh, *sub_bnds[i:i+2]) for i in range(bnd_size - 1)]
    # lengths normalized to 0~1
    lengths = np.array(lengths) / sum(lengths)
    # lengths accumulated
    _acc_lens = np.cumsum(lengths)
    _acc_lens -= _acc_lens[0]
    # mapping lengths to (-scale/2, scale/2) and (scale/2, scale/2)
    _x = _acc_lens * scale - scale / 2
    _y = np.ones(bnd_size) * scale / 2
    # combine x and y zip to list
    return np.array(list(zip(_x, _y)))

def __rotate_90(pnts: list, times: int) -> list:
    # pnts is ndarray shape (n, 2)
    # rotate theta angle around (0, 0)
    rot_mat = np.array([[0, -1], [1, 0]])
    for _ in range(times):
        pnts = np.dot(pnts, rot_mat)
    return pnts

'''
*** initial weights ***
'''
def initialize_weights(msh: Trimesh, inn_verts: list, bnd_verts: list) -> csc_matrix:
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
    # str_mesh.remove_degenerate_faces()
    str_mesh.remove_unreferenced_vertices()
    str_mesh.fill_holes()
    str_mesh.fix_normals()

    return str_mesh

'''
build str mesh by sample vertices on param mesh
'''
def build_str_mesh(uns_mesh: Trimesh, param_mesh: Trimesh, sample_nums: int=50, scale: float=2.) -> Trimesh:
    assert sample_nums > 2, 'sample_nums too small'
    # flatten numpy elements to list will accelerate in cycle
    sample_pnts = [
        [scale * ir / (sample_nums - 1) - scale / 2, scale * ic / (sample_nums - 1) - scale / 2, 0.]
        for ir in range(sample_nums) for ic in range(sample_nums)
    ]
    return build_str_mesh_custom(uns_mesh, param_mesh, sample_pnts, scale)
