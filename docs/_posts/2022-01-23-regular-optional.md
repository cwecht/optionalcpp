---
layout: post
title:  "A regular optional"
date:   2022-01-23 17:10:00 +0100
---

## Generic Programming

In the [last post]({% post_url 2022-01-02-class-invariants %}#a-first-very-simple-solution)
we already acknowledged, that optional should support as many programming paradigms supported by C++ as possible.
One of the most important and most frequently (although maybe unknowingly) used programming paradigms is 
[generic programming](https://en.wikipedia.org/wiki/Generic_programming). It is the foundation of the Standard Template
Library (STL) -- namely the containers/data structures and algorithms (for a quick introduction to STL, I'd recommend
[this short video](https://www.youtube.com/watch?v=ltBdTiRgSaw) to you). For now we can think of this paradigm as a
methodology for implementing *generalized* data structures and algorithms using C++ templates. '*generalized*' in this
context means, that the data structures and algorithms are implemented based on generic programming should be usable in
combination with as many types as possible -- including *build-in* types of the language or *user defined types* like
classes or structs.

**Note:** This description of generic programming is oversimplified (the methodology comprises more aspects) and too
concrete (generic programming is not limited to C++), but it serves our purposes for now.
If you want to learn more about generic programming [this talk](https://www.youtube.com/watch?v=1-CmNNp5eag)
by Alexander Stepanov -- the inventor of STL -- or his lecture on
[Efficient Programming with Components](https://www.youtube.com/watch?v=aIHAEYyoTUc&list=PLHxtyCq_WDLXryyw91lahwdtpZsmo4BGD)
is worth a look.

Let's take for example a popular port of the STL: `std::vector<T>`. We can instantiate it with build-in
types like integers (`std::vector<int>`) or a more complex type like `std::string` (`std::vector<std::string>>`).
We also can pass e.g. a vector of integers to `std::sort` and it will sort the vector without any issues.

{% highlight cpp %}
std::vector<int> m = {5, 1, 8};
std::sort(m.begin(), m.end());
// m == {1, 5, 8}
{% endhighlight %}

There are many more useful algorithms in STL -- if you don't already know them, 
[should start learn them now!](https://www.youtube.com/watch?v=2olsGf6JIkU) as the can improve your code significantly.


## Regular Types
But, in order to be used with STL algorithms and containers, a type must support specific operations. For example, in
order to be used with `std::sort`, the element type of a vector (the `T` of `std::vector<T>`) should provide a
`<` (less-than) operator, otherwise we need to pass a comparator function to `std::sort` as well. `int` has such an
operator build-in to the language and `std::string` defines one. Additionally the type must be assignable at least;
it must be possible to assign a vector to another vector of the same type. Otherwise it would not be possible to change
the position of the values in the vector, because `std::vector` works
[in-place](https://en.wikipedia.org/wiki/In-place_algorithm).

STL algorithms and containers are designed in such a way that they will work with as many types as possible -- they
are *generalized* as much as possible. In order to
archive that, they are limited to a number of operations they may require the used types to support. The concrete
set of required operations may differ from algorithm/container to another, but there is a particular set of operations, which is
sufficient for a type to be usable with all STL algorithms (at in the original STL implementation). If a type supports
all of these operations, it is called *regular*. According to "Fundamentals of Programming" -- a seminal book about
generic programming, a reduced version can be found [here](http://stepanovpapers.com/DeSt98.pdf) -- these
operations are:

| Operation | C++ Syntax |
|-----------|------------|
| default constructor | `T a;` |
| copy constructor | `T a = b;` |
| assignment | `a = b;` |
| equality | `a == b` |
| inequality | `a != b` |
| ordering | `a < b` |

If a type supports all these operations, we can expect, that it will work with the STL algorithms and containers.
Typical examples of *regular types*, which are part of the C++ Standard are:

* build-in types such as `int`, `long`, `char`, `float`, `double`, etc.
* `std::string`
* `std::vector<T>` (for a regular `T`)
* `std::map<K, T>` (as well as `std::set<T>`) (for regular `T` and `K`)

Please note, that the fact of containers such as `std::vector<T>` and `std::map<T>` being  regular implies, that we
can nest them as we like, e.g. `std::map<std::string, std::vector<int>>` or even `std::map<std::string, std::map<int,
string>>`. Alexander Stepanov -- the inventor of STL -- once said 
([in this talk](https://www.youtube.com/watch?v=1-CmNNp5eag)), that before STL no one would have thought about using a
[red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree) with read-black trees as elements. With STL this
is easily possible -- for bad or good.

Easier use of a type is one argument for making it *regular*.
[C.11](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c11-make-concrete-types-regular) also states,
that "regular types are easier to understand and reason about". Regular types are behaving just like build-in types
or many types provided by  STL, they are "nothing special", meaning that you do not have to keep special behavior in mind while
using them. You can treat them as any other regular type.

## Conclusion

Therefore we should make sure that our optional is a *regular type*. If not, our optional may not be usable with at
least some parts of STL, which would be unfortunate. In order to do so we will make sure that our optional supports the above
mentioned operations in the upcoming blog posts, staring with the *equality* and *inequality* comparison operators.

