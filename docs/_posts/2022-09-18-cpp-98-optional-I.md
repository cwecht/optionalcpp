---
layout: post
title:  "An C++ 98 Optional: Part I - Groundwork"
date:   2022-09-18 19:00:00 +0100
---

## Where we left C++98

Since we [introduced a union]({% post_url 2022-05-27-enable-non-default-constructed-types %}) in our optional, we had
to move from C++98 to C++11 because C++98 does not support
[unrestricted unions](https://www.programmerall.com/article/940842653/). But since back then we learned, that we can use the
[placement new operator]({% post_url 2022-05-29-copy-constructor-strikes-back %}#making-a-conditional-copy---attempt-ii)
to construct any object at a given address. This knowledge gives us the opportunity to implement a
[minimal core optional]( {% post_url 2022-05-27-enable-non-default-constructed-types
%}#2022-06-26-copy-assignment-revisited) in C++98.

But before we approach this matter, we should lay the foundation first. There are two things we can do to make the
change easier and there is one necessary change unrelated to the actual issue.

## Preliminary step I - Less direct access to `mValue`

It is already pretty much clear, that we won't be able to access the value optional just like any normal member variable
using `mValue`. Therefore we should limit the use of `mValue` in our code to the bare minimum. We can use optional's
dereference operator for that.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  optional(const optional& other)
      : mHasValue(other.mHasValue) {
    if (other.mHasValue) {
      new (&mValue) T(*other);
    }
  }

  optional& operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      *(*this) = *other;
    } else if (other.mHasValue) {
      new (&mValue) T(*other);
    } else if (mHasValue) {
      mValue.~T();
    }
    mHasValue = other.mHasValue;
    return *this;
  }
  //...

  const T& value() const {
    throwInCaseOfBadAccess();
    return *(*this);
  }

  T& value() {
    throwInCaseOfBadAccess();
    return *(*this);
  }
  //...

  const T* operator->() const {
    return &(*(*this));
  }

  T* operator->() {
    return &(*(*this));
  }

  //...
  friend bool operator==(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return *a == *b;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  template <typename U>
  friend bool operator==(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return false;
    }
    return *a == b;
  }
  //...

  friend bool operator<(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return *a < *b;
    }
    return !a.mHasValue && b.mHasValue;
  }

  template <typename U>
  friend bool operator<(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return true;
    }
    return *a < b;
  }

  template <typename U>
  friend bool operator<(const U& a, const optional& b) {
    if (not b.mHasValue) {
      return false;
    }
    return a < *b;
  }
  //...
};

{% endhighlight %}

With these changes, `mValue` is less frequently used in the code, which makes replacing it much easier.

## Preliminary step II - Factor out value construction and deconstructions

If we have a closer look at our code, we will see, that `T`s copy constructor and its destructor are called in multiple
places. We should factor them out into separate functions so that we only need to change them at one place each instead
of multiple.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  optional()
      : mHasValue(false) {}

  optional(const T& value)
      : mHasValue(true) {
    constructValue(value);
  }

  optional(const optional& other)
      : mHasValue(other.mHasValue) {
    if (other.mHasValue) {
      constructValue(*other);
    }
  }

  optional& operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      *(*this) = *other;
    } else if (other.mHasValue) {
      constructValue(*other);
    } else if (mHasValue) {
      destructValue();
    }
    mHasValue = other.mHasValue;
    return *this;
  }

  ~optional() {
    if (mHasValue) {
      destructValue();
    }
  }
  //...
 
 private:
  //...

  void constructValue(const T& other) {
    new (&mValue) T(other);
  }

  void destructValue() {
    mValue.~T();
  }
};
{% endhighlight %}

Please note, that we also removed `mNoValue` from the default constructors initializer list. Initializing the union to
`mNoValue` has never been necessary, but it made obvious, that `mValue` is *not initialized* in the default constructor.
As soon as we remove the union, `mNoValue` will be gone, too, so we would be required to remove it anyways soon.

## Preliminary step III - Remove the `explicit` keyword 

A second preliminary step is to remove the `explicit` key word from the conversion-to-`bool`-operator, which is not
supported in C++98. As this keyword is
[not necessary anymore]({% post_url  2022-08-27-more-efficient-comparisons %}#conclusion), we can remove it for this step back to C++98.

## Conclusion

This times, the changes weren't spectacular or exiting in any way, but these changes were necessary to simplify our
transition away from the union as the underlying mechanism for our optional. Now we focus on these parts of
the code for the upcoming changes:

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  const T& operator*() const {
    return mValue;
  }

  T& operator*() {
    return mValue;
  }
  //...

 private:
  //...
  struct NoValue {};
  union {
    NoValue mNoValue;
    T mValue;
  };
  //...

  void constructValue(const T& other) {
    new (&mValue) T(other);
  }

  void destructValue() {
    mValue.~T();
  }
};

{% endhighlight %}

## PS: Don't forget the tests!

Actually, we managed to use C++11 features in our tests without noticing it:

* braced initialization,
* `>>` instead of `> >` in templates,
* list-initialization for `std::vector` and
* local types in template instantiations.

For the sake of completeness the necessary adaptions are listed here as well.

### Braced initialization

In C++11 we could used *curly* brackets (`{}`) for calling constructors.

{% highlight cpp %}
const optional<CheckedDestructorCalls> x{CheckedDestructorCalls{}};
{% endhighlight %}

[ES.23](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-list) of the C++ Core Guidelines advises to use
them (for very good reasons), but in C++98, they are just not available. We must go back to the old fashioned syntax using *round* brackets
(`()`).

{% highlight cpp %}
const optional<CheckedDestructorCalls> x(CheckedDestructorCalls());
{% endhighlight %}

This compiles -- but the tests are failing. If turns out, that this is a good example of the
[most vexing parse](https://en.wikipedia.org/wiki/Most_vexing_parse). Let's have a look at the whole test.

{% highlight cpp %}
TEST_CASE("An optional with a value destructs the value during destruction.") {
  {
    const optional<CheckedDestructorCalls> x(CheckedDestructorCalls());
    REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 1);
  }
  REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 0);
}
{% endhighlight cpp %}

The test fails, because the first assertion fails: `missingDestructorCalls` equals 0 instead of 1.
This implies, that no constructor has been called at this point. After I encountered this issue, I was thinking:
"Well, so if the constructors are not called, let's check if the optional stores a value!", so I added a
`REQUIRE(x)`. This resulted in a linker error:

{% highlight bash %}
undefined reference to `x(CheckedDestructorCalls (*)())'
{% endhighlight bash %}

This tells ous, that the compiler interprets the definition and initialization of the optional as a *function
declaration*. `CheckedDestructorCalls()` is interpretec as a *function pointer*. hence `x` is interpreted as the
delclaration of a function, which takes such a function pointer and returns an optional. Note that, the linker error
does not mention the "return value" of the aleged function. This is because the linker doesn't know it. It is not
encoded in the [symbol of the function](https://en.wikipedia.org/wiki/Name_mangling).

The [Wikipedia article about the most vexing parse](https://en.wikipedia.org/wiki/Most_vexing_parse) mentions exactly
this issue and suppgests multiple fixes for it. We will stick with copy initialization.

{% highlight cpp %}
TEST_CASE("An optional with a value destructs the value during destruction.") {
  {
    const optional<CheckedDestructorCalls> x =
        optional<CheckedDestructorCalls>(CheckedDestructorCalls());
    REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 1);
  }
  REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 0);
}
{% endhighlight cpp %}

This whole issue shows us one reason for prefering `{}` over `()` for initialization: avoiding the most vecing parse.

### Nicer Template Syntax

In C++98 the token `>>` is *always* interpreted as the
[right shift operator](https://en.cppreference.com/w/cpp/language/operator_arithmetic) -- even in cases in which it is
obviously not intended to be a shift operator like template instantiations. Because of the reason, the following code
will not compile although it is perfectly fine C++11 code.

{% highlight cpp %}
optional<std::vector<int>> x;
{% endhighlight %}

In order to fix this, we need to add one space so that `>>` becomes `> >`.

{% highlight cpp %}
optional<std::vector<int>> x;
{% endhighlight %}

###  List-initialization for `std::vector`

In C++11 we could initialize a `std::vector` -- just like a built-in array -- using 
[list initialization](https://en.cppreference.com/w/cpp/language/list_initialization).

{% highlight cpp %}
std::vector<int> anyValueX = {1, 2, 3};
{% endhighlight %}

This features has been introduced with C++11: `std::initializer_list` has been added to C++ standard, so that the
respective  constructors can now be written. In C++98 we need to add the elements manually to the `std::vector`.

{% highlight cpp %}
std::vector<int> anyValueX;
anyValueX.push_back(1);
anyValueX.push_back(2);
anyValueX.push_back(3)
{% endhighlight %}

### Function local types in templates

In C++98 and C++03 it was not allowed to use
[function local types in templates](https://stackoverflow.com/questions/3470189/why-cant-templates-take-function-local-types).
This is the reason why we can not continue using function local structs in our tests like this:

{% highlight cpp %}
TEST_CASE("An non-const optional's value can be mutated.") {
  struct A {
    unsigned int x;
  };
  unsigned int anyValue = 10;
  A anyStructValue = {anyValue};
  const optional<A> a(anyStructValue);
  //...
{% endhighlight %}

Thankfully, this can be easily fixed by moving the definition of this struct out of the test case definition.
