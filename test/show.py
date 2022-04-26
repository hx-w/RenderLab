import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# offset:
#   face-1: 2
#   face-2: 7
#   face-3: 13
#   face-4: 19
def read_data(filename, offset: int, mindis=False, header=True) -> list:
    pnt = [[], [], []]
    dm = {
        'face1': 2, 'face2': 7, 'face3': 13, 'face4': 19
    }
    with open(filename, 'r') as f:
        for line in f:
            if header:
                header = False
                continue
            elements = line.split(',')
            if mindis and dm[elements[23].strip('"')] != offset:
                continue
            pnt[0].append(float(elements[offset].strip('"')))
            pnt[1].append(float(elements[offset+1].strip('"')))
            pnt[2].append(float(elements[offset+2].strip('"')))
    return pnt

MAXROW = 1
MAXCOL = 2
def plotSub(index: int, color: str, title: str, points: list):
    ax = plt.subplot(MAXROW, MAXCOL, index, projection='3d')
    ax.scatter(points[0], points[1], points[2], c=color, s=1)
    ax.set_zlabel('Z')
    ax.set_ylabel('Y')
    ax.set_xlabel('X')
    plt.title(title)

def plotFace(index: int, title: str):
    dm = {
        'face-1': [2, 'r'],
        'face-2': [7, 'g'],
        'face-3': [13, 'b'],
        'face-4': [19, 'y']
    }
    plotSub(index, dm[title][1], title, read_data('table.csv', dm[title][0]))

def plotMindis(index: int):
    plotSub(index, 'g', 'mindis', read_data('table.csv', 7, True))
    plotSub(index, 'y', 'mindis', read_data('table.csv', 19, True))

plotFace(1, 'face-1')
plotFace(1, 'face-2')
plotFace(1, 'face-3')
plotFace(1, 'face-4')

plotMindis(2)

plt.show()