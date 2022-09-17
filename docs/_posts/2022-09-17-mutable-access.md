---
layout: post
title:  "Mutable Access"
date:   2022-09-17 19:29:00 +0100
---

## A matter of `const`

`optional` provides -- naturally -- access to the value it stores. We have
[multiple ways]({% post_url 2022-07-17-pointer-syntax %}) to do so, but they all lack one capability.

Let's say, we store a `std::vector<int>` in our `optional` and we want to push a value in it.

{% highlight cpp %}
optional<std::vector<int>> vs;
vs->push_back(5);
{% endhighlight %}

This code won't compile. The error message will look basically like this:

{% highlight bash%}
passing ‘const std::vector<int>’ as ‘this’ argument discards qualifiers
{% endhighlight %}

We have seen such an error [before]({% post_url 2022-03-20-const %}). The issue is, that the `->` operator returns a
pointer to a `const T` -- in this case a `const std::vector<int>`. We then try to call `push_back` on this object, which
is rejected, because [push_back](https://en.cppreference.com/w/cpp/container/vector/push_back) is not a `const` member
function.

This means that, if we want to be able to write code as done above, we will need to provide *mutable access* to an
optional's value. So we need to provide non-const versions of

* `value()`
* the `*` operator and
* the `->` operator.

## Adding the tests

Creating a suitable test is pretty straight forward:

{% highlight cpp %}
TEST_CASE("An non-const optional's value can be mutated.") {
  struct A {
    unsigned int x;
  };
  A anyValue = {10};
  optional<A> x(anyValue);
  A anyOtherValue = {5};
  SECTION("value()") {
    x.value() = anyOtherValue;
  }
  SECTION("*-operator") {
    *x = anyOtherValue;
  }
  SECTION("->-operator") {
    x->x = anyOtherValue.x;
  }
  REQUIRE(x->x == anyOtherValue.x);
}

{% endhighlight %}

catch's [SECTIONs](https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md#test-cases-and-sections) come in
quite handy here. Each of the `SECTION`s we introduced here are executed independently although they share the same
setup and assertion code.

## Adding mutable access

The implementation is rather simple, too: we basically can copy the `const` versions of the accessor member functions
and operators and remove the `const` qualifiers from the signature.

{% highlight cpp %}
template <typename T>
class optional {
 public:
 //...
  const T& value() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
    return mValue;
  }

  T& value() {
    if (not mHasValue) {
      throw bad_optional_access();
    }
    return mValue;
  }

  const T& operator*() const {
    return mValue;
  }

  T& operator*() {
    return mValue;
  }

  const T* operator->() const {
    return &mValue;
  }

  T* operator->() {
    return &mValue;
  }
  //...
};
{% endhighlight %}

This is enough to make the new test pass. It is not that nice though that we have to 
[repeat ourselfs](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself) that much. For the operators it is not that bad
after all, but for `value()` it is just ridiculous. Luckily we can help ourselfs by introducing a little helper function.

{% highlight cpp %}
template <typename T>
class optional {
 public:
 //...
  const T& value() const {
    throwInCaseOfBadAccess();
    return mValue;
  }

  T& value() {
    throwInCaseOfBadAccess();
    return mValue;
  }
  //...
  private:
  //...
  void throwInCaseOfBadAccess() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
  }
};
{% endhighlight %}

## Conclusion

With these changes our `optional` can now finally be mutated via `value()`, `*` and `->`. This is great, as it allows
for many more use cases. Additionally it should not be on us -- library implementers -- to decide whether is should be
allowed for a used to change the value of an optional of not. With this solution, the user can make this decision on
it's own simply but putting a `const` on the respective `optional` or not.
