# -*- coding: utf-8 -*-

'''
@desc export funcs:
    1. 'get_project_files': recieve project_path, get target_files(3D mesh files)
    2. 'update_config': recieve project_path, target_files, update config file and create output folder

@date 2023.01.12
'''


import os
from typing import List
import datetime
import hashlib
import logging
import toml

config_name = '.rdlab.toml'
output_folder = 'static'

# setup logger profile
logging.basicConfig(
    level=logging.DEBUG,
    format='[%(levelname)s]: %(message)s'
)


'''
@brief project_files must be 3D mesh files (*.obj, *.stl, ...)
       and either with 4 files or 2 files (CBCT face1-4, IOS face1,4)

@return target_files: List[str], sorted by asc
'''
def get_project_files(project_path: str) -> List[str]:
    # get all files in project_path
    files = []
    for _, _, fs in os.walk(project_path):
        # only first layer
        files = list(filter(lambda f: f.endswith('.obj') or f.endswith('.stl'), fs))
        break
    # sort files
    files.sort()

    if len(files) in [2, 4]:
        return files
    return []


'''
@brief config file at '${project_path}/.rdlab.toml

[meta]
create_time = "%Y-%m-%d %H:%M:%S"
modify_time = "%Y-%m-%d %H:%M:%S"

hash: int

[source]
files = []
output = "./output"

@return success: bool, if False, shows source file changed
'''
def update_config(project_path: str, target_files: List[str], force: bool = False) -> bool:
    # get current time %Y-%m-%d %H:%M:%S
    now = datetime.datetime.now()
    now_str = now.strftime("%Y-%m-%d %H:%M:%S")
    # hash files
    hash_value = 0
    for file in target_files:
        hash_value ^= hash_file(os.path.join(project_path, file)) & 0xdeadbeef
    # check file exists
    config_full = os.path.join(project_path, config_name)
    if os.path.exists(config_full):
        # read config file
        config = toml.load(config_full)
        # check hash
        if force or config['meta']['hash'] == hash_value:
            if force and config['meta']['hash'] != hash_value:
                # clear workflow cache if force and hash changed
                config['workflow']['node_order'] = []
                config['workflow']['context'] = {}
                config['meta']['hash'] = hash_value

            config['meta']['modify_time'] = now_str
            config['source']['files'] = target_files
            config['source']['output'] = output_folder
        else:
            # !!! source file changed, should notify user
            logging.error(f'origin hash {config["meta"]["hash"]} != current hash {hash_value}')
            return False
    else:
        # create config file
        config = {
            'meta': {
                'create_time': now_str,
                'modify_time': now_str,
                'hash': hash_value
            },
            'source': {
                'files': target_files,
                'output': output_folder
            },
            'workflow': {
                'node_order': [],
                'context': {
                    '8': {
                        'Ensure manifolds': True,
                        'Auto fix position': True
                    },
                    '32': {
                        'Samples U': 5,
                        'Samples V': 3,
                        'Assists': True,
                        'Remesh U': 100,
                        'Remesh V': 100,
                        'Weights': 'auto'
                    },
                    '128': {
                        'Remesh U': 100,
                        'Remesh V': 100,
                    }
                }
            }
        }
    # write config file
    with open(config_full, 'w') as f:
        toml.dump(config, f)
    
    # create output folder if not exists
    if not os.path.exists(os.path.join(project_path, output_folder)):
        os.mkdir(os.path.join(project_path, output_folder))

    return True

'''
@brief get hash value of file (sha1)
'''
def hash_file(filepath: str) -> int:
    try:
        with open(filepath, 'rb') as f:
            sha1obj = hashlib.sha1()
            sha1obj.update(f.read())
            hash_value = sha1obj.hexdigest()
            return int(hash_value, 16)
    except Exception as e:
        logging.error(f'hash error {e}')
        return 0


if __name__ == '__main__':
    ''' only for test
    '''
    project_path = 'testcase'
    target_files = get_project_files(project_path)
    print(update_config(project_path, target_files, True))