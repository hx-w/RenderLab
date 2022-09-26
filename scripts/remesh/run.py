# -*- coding: utf-8 -*-

import args
import validate

if __name__ == '__main__':
    _args = args.parse_args()

    if _args.validate:
        print('[PY] validating mesh')
        validate.validate(_args.input, _args.output)

