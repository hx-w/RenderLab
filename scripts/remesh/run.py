# -*- coding: utf-8 -*-

import args
import validate
import remesh

if __name__ == '__main__':
    _args = args.parse_args()

    if _args.validate:
        print('[PY] validating mesh')
        validate.validate(_args.input, _args.output)

    if _args.remesh:
        print('[PY] remeshing mesh')
        remesh.convert(_args.input, _args.output, _args.pivots)
