## 2023/01/18

- [x] 在header中的前向声明只能用作指针，如果直接定义变量，会报错C2079
- [x] `bind(&CLASS::method, inst_ptr, ::placeholders::_1, ...)` 中，如果`inst_ptr`为智能指针，只有作为`unique_ptr`时有效，`shared_ptr`无效，时候`inst_ptr.get()`得到空指针

- [x] 在GUI模块中为了方便修改`Drawable`属性，我直接从render模块获取了对应的元素指针。这样做可能会有数据不一致的风险

## 2023/01/24

- [x] 导入项目绝对路径如果有中文会出问题

- [x] `toolkit.cpp`中全局有一个py解释器，第一次调用py的时候又创建了一个解释器，这样居然能解决问题。

