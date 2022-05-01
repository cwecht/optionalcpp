---
layout: post
title:  "Pass by Value vs. Pass by Reference"
date:   2022-05-01 19:20:00 +0100
---

## Passing by Value

As of now we always passed our optional or its value type (which has been `unsigned int`) around _by value_.
This means, that, as soon as we passed our optional or a value of its value type to a (member) function or returned one of them
from a function, we created a copy. Every time.

{% highlight cpp %}
template<typename T>
class optional {
 public:
  // ...
  // passing T by value
  optional_unsigned_int(T value) {
  //...
  // returning T by value
  T value() const {
  //..
  // passing optional<T> by value
  friend bool operator==(optional a, optional b) {
  //...
};
{% endhighlight %}

As long as `mValue` had the type `unsigned int`, this was fine and reasonable. Actually,
[F.15](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f16-for-in-parameters-pass-cheaply-copied-types-by-value-and-others-by-reference-to-const)
of the C++ Core Guidelines advises us to "cheaply-copied types by value" to functions and a struct holding an integer
and a boolean can be considered to be cheaply-copied on most architectures.

But what if the `T` in `optional<T>` is now something else, let's say a `std::string`? Such a string might be pretty big,
so  -- in general -- we only want top copy them if we must. But passing an `optional<std::string>` to its comparison function is no
such case.

## Copy Counting Tests

If we want to make sure, that no unnecessary copies are made, we should cover the desired copying behavior in our tests.
But how can we do that? A quite simple solution is to write a class which counts every time it is copied.

{% highlight cpp %}
struct CopyCounting {
  // const disallows assignment, so we don't need to overwrite
  // the copy assignment operator.
  int const copyCount;

  // default constructor
  CopyCounting()
      : copyCount(0) {}

  // copy constructor
  CopyCounting(const CopyCounting& other)
      : copyCount(other.copyCount + 1) {}
};
{% endhighlight %}

`CopyCounting` is such a `class`. It has a member `copyCount`, which stores the number of times, this object has been
copied since a first instance was created using the default constructor. Pleas note, that this is not a "global copy
count". If you make two copies of the same object, their copy count will be identical.

{% highlight cpp %}
CopyCount x; // copyCount is 0
CopyCount a(x); // copyCount is 1
CopyCount b(x); // copyCount is 1 and a.copyCount is still 1
{% endhighlight %}

Now we can write a new test, which specifies, how many copies of an optional's value we expect.

{% highlight cpp %}
TEST_CASE(
    "During initialization of an optional with a value and reading from it, "
    "the value type is copied only once.") {
  CopyCounting c;
  REQUIRE(c.copyCount == 0);
  const optional<CopyCounting> x(c);
  REQUIRE(x.value().copyCount == 1);
}
{% endhighlight %}

## Passing by constant reference

For such situations, passing parameters `by const ref` -- meaning "passing by constant reference" -- comes in handy.
We can just replace `T` with `const T&` and `optional` with `const optional&` in all function argument definitions.

{% highlight cpp %}
template<typename T>
class optional {
 public:
  // ...
  // passing T by reference
  optional_unsigned_int(const T& value) {
  //...
  // returning T by value
  const T& value() const {
  //...
  // passing optional<T> by reference
  friend bool operator==(const optional& a, const optional b) {
  //...
 };
{% endhighlight %}

This implementation will make our new test pass.
But why do we use constant references instead of non-constant references? There are several reasons:
* it is considered good practice to grant only mutable access if needed. This way, a reader or user of the function can
  tell only by looking at the signature of the function, that these function can not modify the value of this parameter.
  Many coding guideline recommend to pass an arguments _only_ by a non-constant reference if the function will change
  it (e.g. see
  [F.17](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f17-for-in-out-parameters-pass-by-reference-to-non-const)
  of the C++ Core Guidelines).
* constant references can bind to temporaries. Let's say we have a function `optional<std::string> f()` and we want to
  compare its return value directly with another optional: `f() == optional<std::string>()`. In this case, the return
  value of `f` is a *temporary object*, meaning that it is not assigned to any variable but the compiler still needs to
  store it anywhere. The C++ Standard only allows constant references to bind to such temporaries. This is true not only
  for function parameters but also for references used elsewhere in C++ code. If we'd like to assign `f()`s return value
  to a non-`const` reference, the compiler would not allow that. But for a non-`const` reference, the compiler would allow
  it. The temporaries' lifetime is actually [extended](https://abseil.io/tips/107) as well.
* in case  of `value() const` we have no other choice, as it is not allowed to return a non-`const` reference to a member
  from a `const` member function. Given, that we already learned that a `const` member function's `this` pointer points to
  a `const` [(see here)]({% post_url 2022-03-20-const %}#member-functions-and-constants)
  object, this makes sense, because a constant object's member are constant as well. But we can not take a non-`const`
  reference from a constant, but only a constant reference.

Because of this reasons, we will from now on pass `optional<T>` and `T` objects/values pass by reference to `const`.

## Conclusion

After we had [generalized our optional]({% post_url 2022-04-03-template %}), we saw, that our first naive
generalization had been sub optimal for non-trivial cases. By passing values by constant reference instead of passing
them by value, we saved quite some unnecessary -- potentially expansive -- copies of large values.

But there are still more cases consider. One of them, we will look at in the upcoming post.
