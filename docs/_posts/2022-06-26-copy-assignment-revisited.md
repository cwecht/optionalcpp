---
layout: post
title:  "Copy assignment revisited"
date:   2022-06-26 16:13:00 +0100
---

## Remember the ints

With `int`s we can do something like this.

{% highlight cpp %}
int x = 5;
int y = 0;
int z = 0;
z = y = x;
// both y and z are equal to 5 now.
{% endhighlight %}

If we are trying to do something similar with our optional in a new test case, we will get an compiler error.

{% highlight cpp %}

TEST_CASE("An optional and its copy are equal.") {
  //...
  SECTION("copy assignment to two optionals") {
    int anyValueX = 5;
    optional<int> x(anyValueX);
    optional<int> y;
    optional<int> z;
    z = y = x;
    REQUIRE(x == y);
    REQUIRE(x == z);
  }
  //...
}
//...
{% endhighlight %}

The compiler error will basically look like this:

{% highlight bash %}
/home/user/optionalcpp/tests/tests.cpp: In function ‘void ____C_A_T_C_H____T_E_S_T____14()’:
/home/user/optionalcpp/tests/tests.cpp:154:13: error: no match for ‘operator=’ (operand types are ‘optional<int>’ and ‘void’)
  154 |     z = y = x;
      |             ^
In file included from /home/user/optionalcpp/tests/tests.cpp:2:
/home/user/optionalcpp/include/optional.hpp:22:8: note: candidate: ‘void optional<T>::operator=(const optional<T>&) [with T = int]’
   22 |   void operator=(const optional& other) {
      |        ^~~~~~~~
/home/user/optionalcpp/include/optional.hpp:22:34: note:   no known conversion for argument 1 from ‘void’ to ‘const optional<int>&’
   22 |   void operator=(const optional& other) {
{% endhighlight %}

It is complaining, that there is no conversion from `void` to `const optional<int>&', but what does it mean?

## Copy assignment operator's signature revisited

Let's revisited the signature of our optional's copy assignment operator.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  void operator=(const optional& other) {
  //...
};
{% endhighlight %}

We already saw, that the assignment operator has a member function like
[call syntax]({% post_url 2022-06-18-return-of-the-assignment %}#a-first-naive-approach), meaning that we can transform
`a = b` to `a.operator=(b)`. This implies, that we can our example from above rewrite to:

{% highlight cpp %}
z = y = x;
// is equivalent to
z.operator=(y.operator=(x));
{% endhighlight %}

We are assigning the *return value* of `y.operator=(x)` to `z`, but the return value of optional's assignment operator
is `void`. This explains the compilers error message: `y.operator=(x)` returns `void`, which can not be converted to
`const optional<int>&` -- it wouldn't make any sense anyways.

So, in order to allow this usage of the assignment operator, we need to change it's return value. We need to somehow
return the new value of `y`. We could do it be returning a copy of `y` by value, but this is quite wasteful, this copy
would be temporary and directly destroyed after the second assignment. It makes way more sense to return a reference to
`y`.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  optional& operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      mValue = other.mValue;
    } else if (other.mHasValue) {
      new (&mValue) T(other.mValue);
    } else if (mHasValue) {
      mValue.~T();
    }
    mHasValue = other.mHasValue;
    return *this;
  }
  //...
};
{% endhighlight %}

With this implementation, our new test compiles and succeeds. Note, that we are returning a non-const reference here.
One could argue, that it makes more sense to return a constant reference here.
[F.47](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f47-return-t-from-assignment-operators) of the
C++ Core Guideliens discourages this due to consistency with standard types: e.g. `int`s assignment operator behaves
like it would return a non-const reference. `(a = b) = c` is possible with `int`s but would not be possible, if the
assignment operator would return a constant reference.

## Conclusion

optional should now behave like a `int` copy assignment wise. Actually, it should new behave in the very most cases
like an `int` consistently.

From my point of view, our optional as it is now, can be considered to be "the minimal core" of an optional to be
usfull, having the following characteristics:
1. It encapsulates a `union` and a `bool` making it a
[tagged union]({% post_url 2022-05-27-enable-non-default-constructed-types %}#2022-05-27-enable-non-default-constructed-types).
2. It is a [regular type]({% post_url 2022-01-23-regular-optional %}).

This is the core functionally of an optional. (Almost) everything else is about
[syntactic sugar](https://en.wikipedia.org/wiki/Syntactic_sugar) and optimizations. But I am positive, that implementing
these will be educational and fun anyways.

