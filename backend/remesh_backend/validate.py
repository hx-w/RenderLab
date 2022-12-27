# -*- coding: utf-8 -*-
'''
validate triangle mesh by
 - removing unused vertices
 - removing degenerate triangles
 - removing duplicate triangles
 - removing smaller components (if any)
'''

import trimesh

def validate(inp: str, otp: str):
    mesh = trimesh.load(inp)
    mesh.remove_degenerate_faces()
    mesh.remove_duplicate_faces()
    mesh.remove_unreferenced_vertices()
    mesh = mesh.split(only_watertight=False)[0]
    mesh.export(otp)
