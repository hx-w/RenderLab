# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

class PointList():
    def __init__(self):
        self.X = []
        self.Y = []
        self.Z = []

class PointListTuple():
    def __init__(self):
        self.source = PointList()
        self.target = PointList()

def read_data(filename: str) -> dict: # LIST[point], point: [X, Y, Z]
    point_dict = {}
    with open(filename, 'r') as ifile:
        header = True
        for line in ifile:
            if header:
                header = False
                continue
            elements = line.split(',')
            label = elements[-1].strip('\n')
            if label not in point_dict:
                point_dict[label] = PointListTuple()

            point_dict[label].source.X.append(float(elements[2].strip('"')))
            point_dict[label].source.Y.append(float(elements[3].strip('"')))
            point_dict[label].source.Z.append(float(elements[4].strip('"')))
            point_dict[label].target.X.append(float(elements[5].strip('"')))
            point_dict[label].target.Y.append(float(elements[6].strip('"')))
            point_dict[label].target.Z.append(float(elements[7].strip('"')))

    return point_dict

pdict = read_data("test.csv")

MAXROW = 2
MAXCOL = 2

def display_face(index: int, face: PointList, title: str, color: str):
    ax = plt.subplot(MAXROW, MAXCOL, index, projection='3d')
    ax.scatter(face.X, face.Y, face.Z, c=color)
    ax.set_zlabel('Z')
    ax.set_ylabel('Y')
    ax.set_xlabel('X')
    plt.title(title)

# face-1
display_face(1, pdict['DEFAULT'].source, '', 'r')
display_face(1, pdict['ON_EDGE'].source, '', 'g')
display_face(1, pdict['IN_EDGE'].source, '', 'b')
display_face(1, pdict['SPECIAL'].source, 'face-1', 'black')

# face-1 & face-4
# display_face(2, pdict['DEFAULT'].source, '', 'r')
display_face(2, pdict['ON_EDGE'].source, '', 'g')
display_face(2, pdict['IN_EDGE'].source, '', 'b')
display_face(2, pdict['SPECIAL'].source, '', 'black')

display_face(2, pdict['ON_EDGE'].target, '', 'g')
display_face(2, pdict['IN_EDGE'].target, '', 'b')
display_face(2, pdict['SPECIAL'].target, 'face-1&face-4', 'black')


# face-2
display_face(3, pdict['DEFAULT'].target, 'face-2', 'r')

# face-4
display_face(4, pdict['ON_EDGE'].target, '', 'g')
display_face(4, pdict['IN_EDGE'].target, '', 'b')
display_face(4, pdict['SPECIAL'].target, 'face-4', 'black')

plt.show()
