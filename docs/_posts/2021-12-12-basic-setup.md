---
layout: post
title:  "A basic cmake setup"
date:   2021-12-12 20:54:00 +0100
---

## The Cmake Setup

Before we start with the actual coding, we should spend some time on the build system, we will use. Although there
are not too many source files to manage within this project, we will use cmake, because
["modern cmake"](https://www.youtube.com/watch?v=bsXLMQ6WgIk&t=337s) has a rather declarative syntax and some
features which are helpful in this projects context.
Let's start with the first few lines of our `CMakeLists.txt`.

{% highlight cmake %}
cmake_minimum_required(VERSION 3.0)

project(optionalcpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 98)
{% endhighlight %}

We chose `3.0` as the minimal required version, because it should serve our purposes quite well, so that we don't
need any newer version. But we might need to update the version in order to user more recent versions of Catch2.

The cmake variable `CMAKE_CXX_STANDARD_REQUIRED` has been set to on `ON` because we shouldn't rely on any compiler extensions
like [the gnu extensions](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Extensions.html). The `CMAKE_CXX_STANDARD` has been
set to C++98, because -- as mentioned in [about](/optionalcpp/about) -- we will start to with this very first official
ISO C++ standard, implement as many features as possible (or reasonable) using this standard and progress to newer
standards as soon as it makes sense.

But let's now move on to the parts of the `CMakeLists.txt` which are concerned with actually building our project.

{% highlight cmake %}
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
{% endhighlight %}

Because `optionalcpp` will be a [header-only library](https://en.wikipedia.org/wiki/Header-only), we should use a
`INTERFACE` library target in cmake. This way, we can use our library like every other library (which exports a
cmake target) in [`target_link_libraries`](https://cmake.org/cmake/help/v3.0/command/target_link_libraries.html)
calls even though we don't have any cpp-files to compile. `PROJECT_NAME` is set by the
[project](https://cmake.org/cmake/help/latest/command/project.html#command:project)
command, so it will expand to `optionalcpp` in our case. We will use this variable wherever possible.

Now we will have a look at our tests.

{% highlight cmake %}
add_executable(test_${PROJECT_NAME} tests/tests.cpp)
target_link_libraries(test_${PROJECT_NAME} ${PROJECT_NAME})

target_include_directories(test_${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/submodules/Catch2/include
)
{% endhighlight %}

We will include the 
[Catch](https://github.com/catchorg/Catch2/tree/Catch1.x) unit testing framework as a 
[git submodule](https://www.atlassian.com/git/tutorials/git-submodule), as it's simple and
gives us the flexibility to upgrade to newer versions easily. As we are for now using C++98, we need to use not even
Catch2 but Catch 1.x. We need to add Catch's `include` folder directly to the test executable, because in this version
Catch does not export any cmake targets to which we could link to.

For now we will not use [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html). It give us no real
benefit, as we have only one test executable.

## Build and Run Tests

To build and run the tests, you need to checkout the repository at first. The `--recurse-submodules` will checkout the
submodules (in our case the Catch repository) during cloning.

{% highlight bash %}
git clone https://github.com/cwecht/optionalcpp.git --recurse-submodules
{% endhighlight %}

After that we can navigate in to the repository, create the build folder, build the project and run the tests.

{% highlight bash %}
cd optionalcpp
mkdir build
cd build
cmake ..
cmake --build . && ./test_optionalcpp
{% endhighlight %}

Running the last command will result in our test executable telling our, that no tests have been executed.

{% highlight bash %}
===============================================================================
No tests ran
{% endhighlight %}

But this will change in the upcomming blog post, in which we will finally start with the implementation of optional.
