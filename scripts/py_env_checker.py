# -*- coding: utf-8 -*-

'''
Check py requirement works fine or not
workspace at ${PROJECT_SOURCE_DIR}

@author CarOL
'''

import subprocess
from importlib.util import find_spec


def install_package(package: str, mirror: str = None):
    if mirror:
        subprocess.check_call(['python', '-m', 'pip', 'install', package, '-i', mirror])
    else:
        subprocess.check_call(['python', '-m', 'pip', 'install', package])

'''
@return new_installed: int
'''
def make_requirements_installed(reqs: list, mirror: str = None) -> int:
    return len([install_package(pkg, mirror) for pkg in reqs if not find_spec(pkg)])


if __name__ == '__main__':
    make_requirements_installed(
        ['trimesh', 'toml', 'tqdm', 'rtree'],
        'https://pypi.tuna.tsinghua.edu.cn/simple'
    )
