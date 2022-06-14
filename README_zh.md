## NoneRender

[![CodeFactor](https://www.codefactor.io/repository/github/hx-w/nonerender/badge)](https://www.codefactor.io/repository/github/hx-w/nonerender)

### 构建与运行

将`JawScan.stl`放到`./static/STL/`文件夹下

- **Win32**: `.\build.bat && .\main.exe`
- **MacOS**: `sh build.sh && ./main`

### 说明

- 每个nurbs牙面模型包含4个nurbs面(`face1, face2, face3, face4`)
- `face1` 包含颜色：`white`(默认), `green`(在边缘上), `blue`(在边缘内)
- `face2` 包含颜色：`yellow`
- `face3` 包含颜色：`light blue`
- `face4` 包含颜色：`light green`，`face4`上包含从`face1`映射离散点
- 控制台中会显示详细操作数据

> 默认会读取并渲染`static/sample`、`static/N1`和`static/N2`作为展示
>
> `JawScan.stl`模型仅用于渲染实验

### 使用方法

- `W, A, S, D` 将摄像机前进，左移，后退，右移
- `H` 显示或隐藏所有元素
- `T` 切换渲染模式：线框模式、实体模式、点模式
- `鼠标按下与移动` 旋转摄像机
- `鼠标滚轮滚动` 缩放视角
- `Ctrl + 鼠标左键点击` 拾取`face1`上的点，并且显示该点与`face2`、`face3`最近点的连线
- `Ctrl + R` 清空所有拾取记录
- `ESC` 退出

> **注意**
> 模型放置变换默认使用单位阵，Nurbs牙模型会逐面生成并显示在初始摄像机位置与视角的偏下方

### 三方库

- `src/render/libs` 渲染开源库
- `src/infrastructure` 某实际项目中的架构模型

2022/05/14