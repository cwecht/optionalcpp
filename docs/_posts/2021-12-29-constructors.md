---
layout: post
title:  "Constructors for valid optionals"
date:   2021-12-29 18:54:00 +0100
---

## Why do we need constructors?

Let's have a look again on our current implementation.

{% highlight cpp %}
struct optional_unsigned_int {
  bool is_set;
  unsigned int value;
};
{% endhighlight %}

It does it's job. We can initialize an optional with and without a value.

{% highlight cpp %}
/* initialize optional without a value */
optional_unsigned_int a;
a.is_set = false;

/* initialize optional with value */
optional_unsigned_int b;
b.is_set = true;
b.value = 480;
{% endhighlight %}

If an optional is initialized without a value, only `is_set` is initialized to `false` but `value` is not initialized at
all. This is fine, because anyone who intents to read from an optional is obligated to check `is_set` first before
reading from `value`. This also implies, that, if a optional is initialized with a value, `is_set` must be set to `true`.
Otherwise, the value can never be read by other code, which uses optional correctly as intended.

On the other hand, if `is_set` is set to `true` but `value` is *not* initialized, we have a problem. Let's have a look
at the following example.

{% highlight cpp %}
optional_unsigned_int a;
a.is_set = true;

/* further down the code or in another function */
if (a.is_set) {
   unsigned int x = a.value;
}
{% endhighlight %}

During the initialization of `x` we read from `value` which is uninitialized memory. This may lead to serious problems and
is surely a bug.

The issue is that our current optional implemented as a C-style struct makes such a rather simple error possible.
According to Scott Meyers, [the most important design guideline](https://www.youtube.com/watch?v=TdajK_SXwoc) is:

> Make an interface easy to use correctly and hard to use incorrectly.
>
> -- Scott Meyers

Making the initialization as easy and robust against programming errors as possible is one of the most important and
obvious use cases of *constructors*.

## Which constructors do are needed?

We already saw, that there are two cases of initialization for our optional:
1. the initialization of an optional *without* a value (`is_set` is set to `false`) and
2. the initialization of an optional *with* a value (`is_set` is set to `true` and the value is initialized).

These two cases can be distinguished by the presence of a *value*; the value of `is_set` is derived from whether
there is *a* value (`is_set` is set to `true`) or if there is *no* value (`is_set` is set to `false`) to initialize 
the optional with. Therefore it is reasonable for `optional_unsigned_int` to have one constructor which takes a value
and another constructor, which doesn't.

{% highlight cpp %}
struct optional_unsigned_int {

  optional_unsigned_int();
  optional_unsigned_int(unsigned int v);

  bool is_set;
  unsigned int value;
};
{% endhighlight %}

By using these constructors, one can be sure, that the resulting optional will always be valid in the sense
that `is_set` will always have the correct value after construction. Of cause even if constructed by using one
of those constructors, one might still change `is_set` afterwards and invalidate the optional. But we will tackle
this issue in another post.

## Writing tests first

It is not a particularly hard task to implement these two constructors, but to keep in line with
[test-driven developement](https://en.wikipedia.org/wiki/Test-driven_development) we will implement
out tests first.

{% highlight cpp %}
TEST_CASE("A default constructed value is not set.") {
  optional_unsigned_int x;
  REQUIRE(x.is_set == false);
}

TEST_CASE("An optional constructed with a value is set and stores the value.") {
  unsigned int anyValue = 10;
  optional_unsigned_int x(anyValue);
  REQUIRE(x.is_set == true);
  REQUIRE(x.value == anyValue);
}
{% endhighlight %}

As you can see, the [Catch testing framework](https://github.com/catchorg/Catch2) allows us to give very readable names
to our test cases. In fact, it makes it pretty easy, to choose names, which are complete english
sentences. All of our test names together describe the behaviour of the class/struct: to put it in Kevlin Henneys words
["they are executable specification" ](https://www.youtube.com/watch?v=SUIUZ09mnwM&t=659s).
I really like this approach and we will use it through out this whole project.

## Implement the constructors

If we compile our tests now, we will get a linker error: `undefined reference`. This makes sense because we did only
*declare* the constructors -- we only introduced their names -- but we did not provide a *definition*. In order to
provide a very first implementation and to satisfy the linker, we may just add curly braces (`{}`) at the end of
each constructor. Now we can build our tests successfully. 

If we run our tests now, but all of them fail. Now we can implement our constructors.

{% highlight cpp %}
struct optional_unsigned_int { 

  optional_unsigned_int() 
    : is_set(false) {}

  optional_unsigned_int(unsigned int v)
    : is_set(true)
    , value(v) {};

  bool is_set;
  unsigned int value;
};
{% endhighlight %}

With this implementation all our tests are passing without any complaints.

**Note:** I am aware of [C.45](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c45-dont-define-a-default-constructor-that-only-initializes-data-members-use-in-class-member-initializers-instead) and
[C.48](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c48-prefer-in-class-initializers-to-member-initializers-in-constructors-for-constant-initializers) of the C++ Core Guidelines.
But we stick for now with C++98, which does not support in-class member initializers.
We will modernize our code as soon as we update to C++11.

## Conclusion

Frankly speaking, this post turned out longer than expected, but we had a lot to cover.
1. We had to convince ourselves that we need constructors: they make for easier to use interfaces (and much more as we will see soon).
2. We had to write our very first tests -- along with some clarifications about the used testing framework Catch and our naming convention for test.
3. We had to add our first code, which actually implements something.

Hopefully the upcoming posts will be shorter. But we still need to do a bit of foundational work before we can proceed
with the more interesting topics.


