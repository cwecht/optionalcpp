---
layout: post
title:  "Efficient Comparisons"
date:   2022-08-27 19:52:00 +0100
---

## Heterogeneous Comparisons

As our optional is a [regular type]({% post_url 2022-01-23-regular-optional %}), it implements the comparison operators
(`==`, `!=`, `<`, `<=`, `>` & `>=`) for two optionals of the same type. This is already quite useful, as we saw already.
But our current implementation doesn't only allow comparisons between `optional<T>` instances of the same `T`; it also
supports comparing a `optional<T>` with an instance of `T` directly. This is possible, because a value of `T` can be
converted *implicitly* to `optional<T>` using `optional<T>`s constructor `optional<T>(const T& value)`.

{% highlight cpp %}
optional<int> x(5);
bool is_five = x == 5;  // this line
// is equivalent to this line:
bool is_five = x > optional<int>(5);
{% endhighlight %}

In order to make this happen, the compiler needs to create a temporary `optional<int>`, which implies that `4` has to be
copied into the new temporary `optional<int>`.
For small types, like e.g. integers this is fine. But for larger types like `std::strings`, this approach is quite
wasteful, as copying the `std::string` may result in an heap allocation. For types like `std::vector` without a [small
object optimization](https://riptutorial.com/cplusplus/example/31654/small-object-optimization) this issue becomes even
more apparent.

Thankfully, we can circumvent this conversion, by adding more *overloads* of the comparison operators.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  friend bool operator==(const optional& a, const T& b);
  friend bool operator==(const T& a, const optional& b);
  friend bool operator!=(const optional& a, const T& b);
  friend bool operator!=(const T& a, const optional& b);

  friend bool operator<(const optional& a, const T& b);
  friend bool operator<(const T& a, const optional& b);
  friend bool operator>(const optional& a, const T& b);
  friend bool operator>(const T& a, const optional& b);
  friend bool operator>=(const optional& a, const T& b);
  friend bool operator>=(const T& a, const optional& b);
  friend bool operator<=(const optional& a, const T& b);
  friend bool operator<=(const T& a, const optional& b);
  //...
};
{% endhighlight %}

Please note, that we have to add two new overload for every operator in order to enable make it possible to put values
of `T` on both sides of the operator.

## Testing

During the implementation of the [comparison operators]({% post_url 2022-02-20-order %}) we already added tests for the
comparison between `optional<T>` and `T`. So we don't really need new "functional" tests, but we should check, if now
the unnecessary copies are avoided. Since [values are passed by reference]({% post_url 2022-05-01-value-by-reference %})
we have class for counting copies, but this class won't help us here as this class stores the copy count in the newly
created object. In our case, the newly created object would be an temporary, which we can not access after the fact.

In order to do so, we need to count the number of copies globally. Thankfully such a class is easily written.

{% highlight cpp %}
struct GlobalCopyCounting {
  static int copyCount;

  GlobalCopyCounting() {}

  GlobalCopyCounting(const GlobalCopyCounting&) {
    ++copyCount;
  }
};

int GlobalCopyCounting::copyCount = 0;

bool operator==(const GlobalCopyCounting&, const GlobalCopyCounting&) {
  return true;
}

bool operator<(const GlobalCopyCounting&, const GlobalCopyCounting&) {
  return true;
}

{% endhighlight %}

We need provide the `==` and the `<` operator in order to make our tests compile. With this helper class we can now
implement the new test case. 

{% highlight cpp %}
TEST_CASE(
    "While comparing an optional with a value the value is not copied (because "
    "no temporary is created)") {
  const GlobalCopyCounting ref;
  const optional<GlobalCopyCounting> opt(ref);

  REQUIRE(ref == opt);
  REQUIRE(opt == ref);
  REQUIRE(!(ref != opt));
  REQUIRE(!(opt != ref));
  REQUIRE(ref < opt);
  REQUIRE(opt < ref);
  REQUIRE(ref < opt);
  REQUIRE(opt < ref);
  REQUIRE(!(ref >= opt));
  REQUIRE(!(opt <= ref));
  REQUIRE(!(ref <= opt));
  REQUIRE(!(opt <= ref));

  REQUIRE(GlobalCopyCounting::copyCount == 0);
}
{% endhighlight %}

This new test case obivously fail.

## The implementation

We will start with the `==` and `<` operators and then derive the others from them.
For implementing the `==`, we have to implement only one of them explicitly -- the other one can be implement in terms
of the first one. The implementation is actually quite strait forward:
 1. return false, if the optional has no value.
 2. compare the passed value with the value of the optional otherwise.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  friend bool operator==(const optional& a, const T& b) {
    if (not a.mHasValue) {
      return false;
    }
    return a.mValue == b;
  }  

  friend bool operator==(const T& a, const optional& b) {
    return b == a;
  }
  //...
};
{% endhighlight %}

The implementation of `<` is a bit different: as `<` is not symmetrical, we can not easily implement one variant in
terms of the other without using the `==` operator. We could do that, but that would result in a performance penalty.
Instead, we will provide two different implementations for these two cases. The major difference is in the case in which
the optional has no value. As we already discussed during the implementation of the prior 
[comparison operators]({% post_url 2022-02-20-order %}), an optional *without* a value is always smaller than an optional
*with* a value. Depending on witch side of the `<` the non-optional value is, the return value for the "optional without
a value case" is either:
  * `true`, if the non-optional value is on the right side.
  * `false`, if the non-optional value is on the left side.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  friend bool operator<(const optional& a, const T& b) {
    if (not a.mHasValue) {
      return true;
    }
    return a.mValue < b;
  }

  friend bool operator<(const T& a, const optional& b) {
    if (not b.mHasValue) {
      return false;
    }
    return a < b.mValue;
  }
  //...
};
{% endhighlight %}

Given these operators, we now can implement the others in terms of them.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  friend bool operator!=(const optional& a, const T& b) {
    return !(a == b);
  }

  friend bool operator!=(const T& a, const optional& b) {
    return !(a == b);
  }

  friend bool operator>(const optional& a, const T& b) {
    return b < a;
  }

  friend bool operator>(const T& a, const optional& b) {
    return b < a;
  }

  friend bool operator>=(const optional& a, const T& b) {
    return !(a < b);
  }

  friend bool operator>=(const T& a, const optional& b) {
    return !(a < b);
  }

  friend bool operator<=(const optional& a, const T& b) {
    return !(b < a);
  }

  friend bool operator<=(const T& a, const optional& b) {
    return !(b < a);
  }
  //...
};
{% endhighlight %}

With these implementations all of our tests will succeed again.

## Using heterogeneous comparison operators of the value type

This is clearly an improvement, but there is still an issue.
Let's have a look at this example:

{% highlight cpp %}
optional<std::string> opt("hello");

bool val = opt == "he";
{% endhighlight %}

This code will compile and will work. But a temporary `std::string` must be created as the `==` operators of
`optional<std::string>` either take `optional<std:string>` or `std::string` values as arguments. This wouldn't be an
issue, if `std::string` hadn't overloads to compare `std::string` objects with `const char*`. These overload exist in
order to avoid exactly such kind of unnecessary temporary `std::string` objects. But our current implementation can not
use them.

We can fix this issue, but at first, we need a test for it: we need a helper class, which is only comparable using
heterogeneous comparison operators. With such a class, we can easily write a new test case.

{% highlight cpp %}
struct HeterogenousComparableOnly {
  int x;
  friend bool operator==(HeterogenousComparableOnly a, int b) {
    return a.x == b;
  }
  friend bool operator==(int b, HeterogenousComparableOnly a) {
    return b == a.x;
  }
  friend bool operator<(HeterogenousComparableOnly a, int b) {
    return a.x < b;
  }
  friend bool operator<(int b, HeterogenousComparableOnly a) {
    return b < a.x;
  }
};

TEST_CASE(
    "If an optional with a value is compared with an value of another type "
    "heterogeneous comparisons can be used.") {
  int anyInt = 5;
  HeterogenousComparableOnly anyA = {anyInt};
  optional<HeterogenousComparableOnly> opt(anyA);
  REQUIRE(opt == anyInt);
  REQUIRE(anyInt == opt);
  REQUIRE(!(opt != anyInt));
  REQUIRE(!(anyInt != opt));

  REQUIRE(opt <= anyInt);
  REQUIRE(anyInt <= opt);
  REQUIRE(!(opt < anyInt));
  REQUIRE(!(anyInt < opt));

  REQUIRE(opt >= anyInt);
  REQUIRE(anyInt >= opt);
  REQUIRE(!(opt > anyInt));
  REQUIRE(!(anyInt > opt));
}
{% endhighlight %}

`HeterogenousComparableOnly` can only compared with `int`s but not with instances of it's own type. These tests won't
even compile, because `optional` can not call such comparison operators yet. But the implementation can be easily fixed
by making `optional`s heterogeneous operators *function templates*.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  template <typename U>
  friend bool operator==(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator==(const U& a, const optional& b) {
  //...
  template <typename U>
  friend bool operator!=(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator!=(const U& a, const optional& b) {
  //...

  template <typename U>
  friend bool operator<(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator<(const U& a, const optional& b) {
  //...
  template <typename U>
  friend bool operator>(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator>(const U& a, const optional< b) {
  //...
  template <typename U>
  friend bool operator>=(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator>=(const U& a, const optional& b) {
  //...
  template <typename U>
  friend bool operator<=(const optional& a, const U& b) {
  //...
  template <typename U>
  friend bool operator<=(const U& a, const optional& b) {
  //...
};
{% endhighlight %}

Now, the non-optional value passed to these comparison operators is passed as it is to the underlying comparison
operator (which may be heterogeneous or not). With these operators in the motivating example above the raw string
literal will be passed as a `char` array to `optional`s `==` operator and then decay to a `const char*` as it is passed
to the underlying `==` operator.

## Conclusion

Today, we started with an already existing features (comparisons) and found an potential performance issue (unnecessary
temporaries). We were able to fix this issue through templatization.

Please note, that now the `explicit` keyword on the "conversion to `bool` operator" is now not needed anymore for the
comparisons to work. It has been necessary because in case of e.g. a comparison of an `optional<int>` and an `int` the
compiler could not decide whether it should convert the `optional<int>` to a `bool` or the `int` to an `optional<int>`.
This ambiguity is now gone because one of the heterogeneous comparison operators won't need any of such conversions.
The `explicit` keyword is still useful there, but our tests will compile even without. This means, that we currently
only need to use one C++11 feature:  [unrestricted unions](https://www.programmerall.com/article/940842653/).
