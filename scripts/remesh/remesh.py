# -*- coding: utf-8 -*-

import trimesh
from param import *


def convert(inp: str, otp: str, pivots: list):
    # load mesh
    mesh = trimesh.load(inp)

    # remesh mesh
    str_mesh = remesh_mesh(mesh, pivots)

    # save mesh
    str_mesh.export(otp)


def remesh_mesh(uns_msh: trimesh.Trimesh, pivots: list) -> trimesh.Trimesh: 
    inn_verts, bnd_verts = split_bnd_inn(uns_msh)
    # check pivots all in bnd_verts
    if not all([p in bnd_verts for p in pivots]):
        raise Exception('pivots not in boundary')

    # parameterize boundary
    bnd_verts = bnd_verts[1:] # first == last
    f_B = mapping_boundary(uns_msh, bnd_verts, pivots)
    sp_weights = initialize_weights(uns_msh, inn_verts, bnd_verts)
    # parameterize interior
    f_I = solve_equation(sp_weights, f_B, inn_verts, bnd_verts)
    param_msh = build_param_mesh(uns_msh, inn_verts, bnd_verts, f_I, f_B)
    str_msh = build_str_mesh(uns_msh, param_msh, 100)
    return str_msh
