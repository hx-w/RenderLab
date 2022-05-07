# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

class PointList(object):
    def __init__(self):
        self.X = []
        self.Y = []
        self.Z = []

def read_data(filename: str) -> tuple: # LIST[point], point: [X, Y, Z]
    face1, face2, face4 = PointList(), PointList(), PointList()

    with open(filename, 'r') as ifile:
        header = True
        for line in ifile:
            if header:
                header = False
                continue
            elements = line.split(',')
            face1.X.append(float(elements[2].strip('"')))
            face1.Y.append(float(elements[3].strip('"')))
            face1.Z.append(float(elements[4].strip('"')))
            if elements[-1].strip('\n') == 'face2':
                face2.X.append(float(elements[5].strip('"')))
                face2.Y.append(float(elements[6].strip('"')))
                face2.Z.append(float(elements[7].strip('"')))
            else:
                face4.X.append(float(elements[5].strip('"')))
                face4.Y.append(float(elements[6].strip('"')))
                face4.Z.append(float(elements[7].strip('"')))

    return (face1, face2, face4)

face1, face2, face4 = read_data("test.csv")

MAXROW = 1
MAXCOL = 3

def display_face(index: int, face: PointList, title: str, color: str):
    ax = plt.subplot(MAXROW, MAXCOL, index, projection='3d')
    ax.scatter(face.X, face.Y, face.Z, c=color)
    ax.set_zlabel('Z')
    ax.set_ylabel('Y')
    ax.set_xlabel('X')
    plt.title(title)

display_face(1, face1, 'face1', 'r')
display_face(2, face2, 'face2', 'g')
display_face(3, face4, 'face4', 'b')

plt.show()
