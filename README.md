# 背景

一个C++的入门项目, 用于学习C/C++的基础知识.

项目参照:

* [Barenboim/json-parser: JSON parser in standard C (github.com)](https://github.com/Barenboim/json-parser)
* [wfrest/Json: c++ Json library (github.com)](https://github.com/wfrest/Json)

代码参照上述两个项目所写(抄了一遍), 仅供个人学习参考


# 构建

使用CMake进行构建

```shell
mkdir build && cd build
cmake ..
make
```

json解析测试程序构建:

```shell
make jsonPrintExample
```

> 测试程序简单测试了一个json对象的解析和输出