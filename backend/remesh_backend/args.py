# -*- coding: utf-8 -*-
'''
implement arg parser
'''

import argparse


def parse_args():
    parser = argparse.ArgumentParser(description='triangle mesh remesher')
    parser.add_argument('--input', help='input mesh file')
    parser.add_argument('--output', help='output mesh file')
    parser.add_argument('--validate', action='store_true', help='validate mesh')
    # accept list [int]
    parser.add_argument('--remesh', action='store_true', help='remesh mesh')
    parser.add_argument('--pivots', type=int, nargs='+', help='boundary pivots')

    return parser.parse_args()
