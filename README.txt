**项目结构**

- 资源文件：
    face-1.txt
    face-2.txt
    face-3.txt
    face-4.txt
    四个面的数据，已经转换为程序接受的格式

- 源码：
    demo.cpp             输入face-1的uv 和步长 获得最近点信息
    table.cpp            输入步长 获得face-1中所有离散点的最近点表格(table.csv)
    table_parallel.cpp   并行优化后的算表格程序

- 可执行程序：
    demo.exe
    table.exe
    table_parallel.exe

- 输出文件：
    table.csv 表头中min face为最近面名称，min L为最近面的最近距离