## 2023/01/18

- [x] 在header中的前向声明只能用作指针，如果直接定义变量，会报错C2079
- [x] `bind(&CLASS::method, inst_ptr, ::placeholders::_1, ...)` 中，如果`inst_ptr`为智能指针，只有作为`unique_ptr`时有效，`shared_ptr`无效，时候`inst_ptr.get()`得到空指针

- [x] 在GUI模块中为了方便修改`Drawable`属性，我直接从render模块获取了对应的元素指针。这样做可能会有数据不一致的风险

## 2023/01/24

- [x] 导入项目绝对路径如果有中文会出问题

- [x] `toolkit.cpp`中全局有一个py解释器，第一次调用py的时候又创建了一个解释器，这样居然能解决问题。
- [x] `CMakeLists.txt`中的`target_compile_definitions(${target} XXX -DDEFINE)`默认会置`#define DEFINE 1`，如果单纯只想定义一个空白宏，需要在`-DDEFINE`后面加`=`

## 2023/02/03

`0.7.0-prerelease`

- [x] python中`sys.executable`为当前python可执行文件的路径，如果通过pybind11调用`f'{sys.executable} -m pip install {pkg}'`会导致窗口不断复制


