---
layout: page
title: About
permalink: /about/
---

This blog documents my efforts to implement [a optional in C++](https://en.cppreference.com/w/cpp/utility/optional).
I am writing blog, because I believe that many *advanced* C++ features can be explained using the
example of optional. With *advanced features* I mean such features, which one will (or at least should) not encounter in
a first basic introduction to C++, but which might come up as soon as one tries to write more complex classes
or templates. Optional seems to be a reasonable example to use because
1. It's basic ideas are pretty easy to grasp.
2. Quite a few *advanced* C++ features are needed in order to get a satisfying implementation of optional.
3. As optional is part of the C++17 Standard, it has well specified interface and behavior.

During this endeavor I am trying to follow these rules/constraints:
1. I strive for a standard compliant naming of public member functions. Maybe we will get to a point at some time
   in the future, where this blog may help to understand any other implementation of optional. From time to time, I will
   also provide a bit of explanation or justification, why the interface is designed in a certain way.
2. The development will be [test-driven](https://en.wikipedia.org/wiki/Test-driven_development) using the
   [catch2](https://github.com/catchorg/Catch2/tree/devel) testing framework. Hopefully this will be instructive for
   some readers as well.
3. I started with C++03 and will then later proceed with more recent standards (11, 14, 17, 20,...). I will keep using a
   standards version until nothing useful can be added anymore. Hopefully I can illustrate some of the new features,
   possibilities and (possibly) problems of every new standard.
4. I use cmake as a build system. This is basically about convenience, though.
5. I commit code changes and the related blog post in the same commit. This leads to an unconventional work flow.
   If I want to add something to an older blog post, I'll edit the commit and
   [force push](https://www.golinuxcloud.com/git-push-force-examples/) the changes. As this an unusual project, I am
   quite sure, that this unusual workflow will not cause too many relevant problems.

If you have any feedback, suggestions or critique, please feel free to create an issue on the
[github project page](https://github.com/cwecht/optionalcpp/issues). You can also write me a 
[mail](mailto:wechtc@google.com). 
