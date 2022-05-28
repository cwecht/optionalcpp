---
layout: post
title:  "A new destructor"
date:   2022-05-28 18:25:00 +0100
---

## What we left behind

At the end of our [last post]({% post_url 2022-05-27-enable-non-default-constructed-types %}) we already reckoned, that
hadn't done enough reach feature parity with our implementation before we introduced a `union`.

Before that we could for example instantiated our optional with a `std::string`.

{% highlight cpp %}
optional<std::string> optionalString;
{% endhighlight %}

If we do this with our current implementation, we get a compiler error:

{% highlight bash %}
error: use of deleted function ‘optional<std::__cxx11::basic_string<char> >::~optional()
note: ‘optional<std::__cxx11::basic_string<char> >::~optional()’ is implicitly deleted because the default definition would be ill-formed
error: union member ‘optional<std::__cxx11::basic_string<char> >::\<unnamed union\>::mValue’ with non-trivial ‘std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::~basic_string()`
{% endhighlight %}

A although the  destructor is a [special member function]({% post_url 2022-05-27-enable-non-default-constructed-types %}),
the compiler can not generate it for us in this case. This is because, the recently introduced `union` doesn't have
a destructor, because one of its members has a non-trivial destructors (of thype `std::string` in this case). And if we
think abit about it, this makes totally sense: how would a destructor  of this `union` look like in this case? From the
`union` itself no one -- neither we nor the compiler -- can tell, which member if currently active. But this knowledge
would be crucial, because a desctructor of this `union` would need to call the correct desctructor. In our current
example of `optional<std::string>`, it would be quite dagnerous to call bluntly the destructor of `std::string` every
time, not knowing, if the optional even contains a string.

With `unsigned int` this wasn't an issue, because `unsigned int` is *trivially destructable*, which basically means,
that no destructor call is needed here. If all members of a `union` are *trivially destructable*, the whole union is
*trivially destructable*, hence our optional is (as of now) trivial constructable.

As `std::string` is obviously now *trivially destructable*, it's destructor must be called *before* the lifetime of it's
enclosing object ends (in this case the `union` in our optional).

## Defining an appropriate destructor

Fortunatly, from withing our optional, we can exactly tell if `mValue` is the active member of the optional, so that we
can invoke it's destructor if needed. But before we can do this, we need a test for this behaviour. 

{% highlight cpp %}
struct CheckedDestructorCalls {
  CheckedDestructorCalls() {
    ++missingDestructorCalls;
  }
  CheckedDestructorCalls(const CheckedDestructorCalls&) {
    ++missingDestructorCalls;
  }
  ~CheckedDestructorCalls() {
    --missingDestructorCalls;
  }

  static int missingDestructorCalls;
};

int CheckedDestructorCalls::missingDestructorCalls = 0;

TEST_CASE("An optional with a value destructs the value during destruction.") {
  {
    const optional<CheckedDestructorCalls> x{CheckedDestructorCalls{}};
    REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 1);
  }
  REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 0);
}
{% endhighlight %}

We could have used `optional<std::string>` to this test, but then we could not check very much other than, that the test
compiles. With the above defined class `CheckDestructorCalls`, we can check, that for every instance of
`CheckDestructorCalls`, which had been created, the destructor has been called: for every newly created instance
`missingDestructorCalls` in incremented and for every destroyed instance, `missingDestructorCalls` is decremented.
We can now check, that all instances of `CheckDestructorCalls` have been properly destroyed, after the destruction of our optional,
by checking at the end if `missingDestructorCalls` equals zero.

This effort is necessary, because fixing the compiler error is not enough. This can be done by simply defining an empty
destructor.

{% highlight cpp %}
template <typename T>
class optional {
 public:
 // ...
  ~optional() {}
 // ...
};
{% endhighlight %}

This is dangerous, because if the optional holds a value, `T`s destructor will never be called. The compiler can not
recognize this issue, but our test can: with this implementation our test suite compiles but the test fails. We need the
provide a proper implementation.

{% highlight cpp %}
template <typename T>
class optional {
 public:
 // ...
  ~optional() {
    if (mHasValue) {
      mValue.~T();
    }
  }
 // ...
};
{% endhighlight %}

The actual implementation is quite simple though: we only need to *conditionally* call `T`s destructor `~T`. As we can
see in this example, a destructor can be called explicitly like any other "normal" function using the `.`-syntax. Usually
this is not necessary, because the destructor is called automatically as soon as the end of the enclosing scope is
reached. Actually we should usually never call the destructor explicitly, because it will be called anyways at the end of
the scope, so that it would be invoked twice on the same "object" (officially the object is not there anymore after
calling the destructor), which can lead to unintended behavior.

In this case though, we have have no other choice than calling the destructor explicitly, because the compiler will not
generate any code for that, as we already have seen.

## Conclusion

We have seen, that if we put a *non-trivially-destructable* type in a `union`, we need to take care of calling the
appropriate destructor, because the compiler is not able to do it. By defining our own destructor, our optional can now
be used again with non-trivial types, at least in very basic scenarios.

Of cause, this isn't enough. The [rule of three](https://en.wikipedia.org/wiki/Rule_of_three_(C%2B%2B_programming))
states, that as soon as we define the destructor, we should also consider defining the *copy constructor* and the *copy
assignment operator* as well. We will do exactly this in the upcoming posts.


