# -*- coding: utf-8 -*-

from typing import List, Tuple
import trimesh
from trimesh import Trimesh
import numpy as np


class ToothDepthGenerator(object):
    _topo_shape = (100, 100)  # by default
    _thr = 0.3

    '''
    @param args: [v1, f1, v2, f2, ...] (v: vertices, f: faces)
    @param shape: (row, col) of the meshed topo
    @param thr: threshold of the depth
    '''

    @classmethod
    def generate_cmd(cls, args: List[np.array], shape: Tuple[int, int], thr: float = 0.3) -> np.array:
        assert len(args) % 2 == 0, "args must be even"
        _staff = list(map(lambda x: x.reshape((-1, 3)), args))
        _meshes = [
            Trimesh(vertices=_staff[i], faces=_staff[i + 1])
            for i in range(0, len(_staff), 2)
        ]
        # check _meshes[0] shape valid
        assert _meshes[0].vertices.shape[0] == shape[0] * shape[1], "mesh shape not match"
        cls._topo_shape = shape
        cls._thr = thr

        return cls._process(_meshes)

    @classmethod
    def _process(cls, meshes: List[Trimesh]) -> np.array:
        ...

    @classmethod
    def _compute_nearest(cls, msh_src: Trimesh, msh_dst: Trimesh) -> np.array:
        _, ndist, _ = msh_dst.nearest.on_surface(msh_src.vertices)
        # ndist = msh_dst.nearest.signed_distance(msh_src.vertices)

        return np.absolute(ndist).reshape(cls._topo_shape)  # 2-dim

    @classmethod
    def _compute_depth_p2(cls, depth_p1: np.array, msh_1: Trimesh, msh_4: Trimesh) -> np.array:
        assert depth_p1.shape == cls._topo_shape, "depth_p1 shape not match"
        zero_boundary_index = []
        for row in range(cls._topo_shape[0]):
            nonz = np.nonzero(depth_p1[row, :])[0]
            if len(nonz) == 0:
                continue
            _min, _max = min(nonz), max(nonz)
            if _min > 0:
                zero_boundary_index.append(row * cls._topo_shape[1] + _min)
            if _max < cls._topo_shape[1] - 1:
                zero_boundary_index.append(row * cls._topo_shape[1] + _max)
        
        index_p2 = np.argwhere(depth_p1.flatten() == 0.)
        avg_norm = np.sum(msh_1.vertex_normals[zero_boundary_index], axis=0)
        avg_norm /= np.linalg.norm(avg_norm)
        # vts_p2 shape(n, 1, 3), reshape to (n, 3)
        vts_p2 = msh_1.vertices[index_p2].reshape((-1, 3))

        _dirs = -np.array([avg_norm] * len(vts_p2))

        loc, index_ray, _ = msh_4.ray.intersects_location(ray_origins=vts_p2, ray_directions=_dirs)
        _new_oris = vts_p2[index_ray]
        _dists = np.linalg.norm(_new_oris - loc, axis=1)
        ray_real_ind = np.array(range(cls._topo_shape[0] * cls._topo_shape[1]))[index_p2][index_ray]

        depth_p2 = np.zeros(cls._topo_shape).flatten()
        depth_p2[ray_real_ind] = _dists.reshape(-1, 1)
        return depth_p2.reshape(cls._topo_shape)



class GeneratorML(ToothDepthGenerator):

    @classmethod
    def _process(cls, meshes: List[Trimesh]) -> np.array:
        assert len(meshes) == 2, "meshes must be 2"
        ...


class GeneratorGT(ToothDepthGenerator):

    @classmethod
    def _process(cls, meshes: List[Trimesh]) -> np.array:
        assert len(meshes) == 4, "meshes must be 4"
        f12_ndist = cls._compute_nearest(meshes[0], meshes[1])
        f13_ndist = cls._compute_nearest(meshes[0], meshes[2])
        diff_f3_f2 = np.abs(f13_ndist - f12_ndist)

        # GT p1
        depth_map_p1 = np.where(diff_f3_f2 > cls._thr, f12_ndist, 0.)

        # GT p2 (fill the hole)
        depth_map_p2 = cls._compute_depth_p2(depth_map_p1, meshes[0],
                                             meshes[-1])

        # combine
        depth_map = np.where(depth_map_p1 > 0., depth_map_p1, depth_map_p2)
        return depth_map



if __name__ == '__main__':
    mshes = [trimesh.load(f'N3/face{i}.obj') for i in range(1, 5)]
    args = [[msh.vertices, msh.faces] for msh in mshes]
    # flatten args
    args = [item for sublist in args for item in sublist]
    depth_map = GeneratorGT.generate_cmd(args, (100, 100), 0.3)

    import matplotlib.pyplot as plt
    plt.imshow(depth_map)
    plt.show()
