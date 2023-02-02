# -*- coding: utf-8 -*-

import matplotlib.cm as cm
from matplotlib.colors import Normalize
import numpy as np

from trimesh.curvature import (
    discrete_gaussian_curvature_measure,
    discrete_mean_curvature_measure
)
import trimesh


def py_mesh_palette(data: np.array, style: str, vminmax: tuple = None) -> np.array:
    if not vminmax:
        vminmax = (np.min(data), np.max(data))
    norm = Normalize(vmin=vminmax[0], vmax=vminmax[1])
    cmap = cm.get_cmap(style)
    return cmap(norm(data))


def py_compute_mesh_curvature(verts: np.array, faces: np.array, type: str) -> np.array:
    verts = verts.reshape((-1, 3))
    faces = faces.reshape((-1, 3))
    mesh = trimesh.Trimesh(vertices=verts, faces=faces)
    if type == 'gaussian':
        return discrete_gaussian_curvature_measure(mesh, verts.tolist(), 1e-3)
    elif type == 'mean':
        return discrete_mean_curvature_measure(mesh, verts.tolist(), 1e-3)
    else:
        raise Exception('Unknown curvature type: ' + type)
