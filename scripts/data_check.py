# -*- coding: utf-8 -*-
import struct

__FORMAT_VERSION__ = b'\x01'

with open('./test.data', 'rb') as ifile:
    _buffer = ifile.read(1)
    assert _buffer == __FORMAT_VERSION__
