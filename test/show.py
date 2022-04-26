import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

data = [[], [], []] # x y z

def read_data(filename):
    with open(filename, 'r') as f:
        first = True
        for line in f:
            if first:
                first = False
                continue
            elements = line.split(',')
            headidx = 7
            data[0].append(float(elements[headidx].strip('"')))
            data[1].append(float(elements[headidx+1].strip('"')))
            data[2].append(float(elements[headidx+2].strip('"')))

read_data('table.csv')
x, y, z = data[0], data[1], data[2]
ax = plt.subplot('111', projection='3d')  # 创建一个三维的绘图工程
#  将数据点分成三部分画，在颜色上有区分度
ax.scatter(x, y, z, c='y')  # 绘制数据点

ax.set_zlabel('Z')  # 坐标轴
ax.set_ylabel('Y')
ax.set_xlabel('X')
plt.show()