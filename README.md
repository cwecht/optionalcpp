# optionalcpp

**optional C++** is a blog, which is about writing a [C++ optional
implementation](https://en.cppreference.com/w/cpp/utility/optional) from scratch. You can find the blog
[here](https://cwecht.github.io/optionalcpp/). This repository contains both, the blog and the source code of the
optional implementation.

## Build

Before you are trying to build the project, please make sure, that you checked out the submodules as well, using e.g.
`git submodule update --init`.

```
cd optionalcpp
mkdir build
cd build
cmake ..
cmake --build . && ./test_optionalcpp
```
