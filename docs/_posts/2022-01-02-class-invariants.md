---
layout: post
title:  "An invariant for optional"
date:   2022-01-02 17:54:00 +0100
---

## The issue

Using the constructors we implemented in the
[last post]({% post_url 2021-12-29-constructors %}#implement-the-constructors), we can create *valid*
optionals. But with our current design it is rather easy to *invalidate* such a valid optional afterwards.
Let's have a look at the following example.

{% highlight cpp %}
/* initialize an optional without a value */
optional_unsigned_int a;

/* invalidate a */
a.is_set = true;

/* further down the code or in another function */
if (a.is_set) {
   unsigned int x = a.value;
}
{% endhighlight %}

We can easily invalidate a valid optional without a value by setting `is_set` to `true`; if we do this we encounter the
in the same issue as described in the
[last post]({% post_url 2021-12-29-constructors %}#why-do-we-need-constructors): we may read from 
uninitialized memory.

The main issue here is, that we can modify `is_set` without touching `value`. This leads us to an important concept of
class design: the `class invariant`.

## What is a class invariant ? 

As already stated in the
[last post]({% post_url 2021-12-29-constructors %}#which-constructors-do-are-needed), there is a connection
between the two members `is_set` and `value` of our optional: `is_set` tells us, whether `value` stores a valid value.
Such a connection can usually be described by a *logical condition* called *class invariant*. 

> A class invariant is a logical condition which is
> 1. established by a class' constructor and
> 2. constantly maintained between calls to public methods.

In our particular case we can phrase optional's class invariant like this:

> An optional's value may be read if and only if the optional contains a valid value.

Note that we used this condition already in 
[last post's]({% post_url 2021-12-29-constructors %}#why-do-we-need-constructors) example to illustrate
why an invalid optional is harmful. The class invariant describes the essential properties which a given class has.
It eases *designing* and *developing* a class, since we can assume that the class invariant holds true before every
public member function call, as well as *using* a class, since it describes the class concisely. We can even say that
maintaining its invariant is a class' main (or even only) concern.

A class' members are *coupled* by the class invariant. If a class has an invariant, it usually has
[*high cohesion*](https://en.wikipedia.org/wiki/Cohesion_(computer_science)) as well, which is often an indicator for
good design. This means that a concise class invariants can lead to a good software design consisting of *small*
and *cohesive* classes which are *loosely coupled*. Invariants also help to reason about code. In fact, invariants are
often necessary in order to use [*design by contract*](https://en.wikipedia.org/wiki/Design_by_contract)
(e.g. the [Java Modeling Language](https://en.wikipedia.org/wiki/Java_Modeling_Language)). Such contracts can then be used
for formal verification.

## A first very simple solution

The traditional way of maintaining a class invariant in object-oriented programming is to use
[encapsulation](https://en.wikipedia.org/wiki/Encapsulation_(computer_programming)). In the end we will use this
technique as well, but I want to show you, that there is another possibility: we can just make all our members `const`.

{% highlight cpp %}
struct optional_unsigned_int { 

  optional_unsigned_int() 
    : is_set(false)
    , value(0) {}

  optional_unsigned_int(unsigned int v)
    : is_set(true)
    , value(v) {};

  const bool is_set;
  const unsigned int value;
};
{% endhighlight %}

Note that we need to initializes `value` in every constructor, because the compiler will complain otherwise.
The constructors are making sure, that the class invariant is established correctly (this is basically, what
constructors are good for:
[C.40](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c40-define-a-constructor-if-a-class-has-an-invariant)).
This implementation maintains optional's class invariant, because the invariant is established during construction and can
not be altered afterwards; the above mentioned issue can not occur, because the members can not be modified after
construction.

This solution impresses with simplicity. As this optional implementation is *immutable*, it can be considered a
[purely funcation data structure](https://en.wikipedia.org/wiki/Purely_functional_data_structure). Such data structures
have advantages especially in case of data shared between multiple threads as
[no synchronisation](https://www.youtube.com/watch?v=2yXtZ8x7TXw) is needed (which is one of the reasons why functional
programming languages such as Erlang are quite popular in distributed computing).

But there are reasons why we should not stick with this implementation.
1. C++ is a *multi-paradigm* language. An optional should not only support functional but also e.g.
   procedural programming. Therefore assignment to an optional should be allowed.
2. The solution above is simple because it is *concrete* (as opposed to *generic*) and uses a *build-in* type for
   `value`. Soon we will generalize `optional_unsigned_int` to `optional<T>`. For a
   [*non-trivial*](https://en.cppreference.com/w/cpp/types/is_trivial) type `T`, it might be undesirable to construct a
   value of type `T` for optionals without a value (because constructing a `T` might be expansive or may have side
   effects). We will elaborate this issue in a future post.
3. An optional should be [swappable](https://en.cppreference.com/w/cpp/named_req/Swappable) without copying and
   *movable* (from  C++11 on). As this leads to instances of optional being modified, optional must be mutable to allow
   these operations. For our current `optional_unsigned_int` this concern doesn't matter, but for a *generic* 
   `optional<T>` its important to use these operations for performance reasons, if `T` is hard to copy but easy to move.

## A more suitable solution

So well will use encapsulation then. First we will -- in order with rule
[C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c2-use-class-if-the-class-has-an-invariant-use-struct-if-the-data-members-can-vary-independently)
of the C++ Core Guidelines -- make `optional_unsigned_int` a class and we will -- in order with rule
[C.9](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c9-minimize-exposure-of-members) --  make its
members private and introduce getter-like member functions.

{% highlight cpp %}
class optional_unsigned_int { 
public:
  optional_unsigned_int() 
    : mHasValue(false) {}

  optional_unsigned_int(unsigned int value)
    : mHasValue(true)
    , mValue(value) {};

  bool has_value() {
    return mHasValue;
  }

  unsigned int value() {
    return mValue;
  }

private:
  bool mHasValue;
  unsigned int mValue;
};
{% endhighlight %}

The names of the member functions `has_value()` and `value()` are taken from `std::optional`. Additionally I renamed the
member variables because I had to rename `value` anyways in order to avoid name clashes. Of cause the tests must be
adapted accordingly. It makes not much sense to add new tests though, because we did not implement new behavior of the class.

**Note:** [*const correctness*](https://isocpp.org/wiki/faq/const-correctness#overview-const) is not yet considered. We
will introduces const correct member functions in an upcoming post.

## Conclusion

Finally `optional_unsigned_int` is a class instead of a struct. We made it a class, because 
it must maintain its *class invariant*. We choose to use *encapsulation* and member functions over immutability to
enable a broader use of optional.

In the upcoming posts we will talk a bit more about some general desirable properties of an optional. After that, we can start to generalize our optional.
