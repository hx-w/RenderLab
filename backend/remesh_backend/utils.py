# utils for parameterization

from trimesh import Trimesh
import numpy as np

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
