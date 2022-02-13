---
layout: post
title:  "Equality and Inequality"
date:   2022-02-13 17:15:00 +0100
---

## Equality Comparison for Optional

We will start with the equality comparison, namely the `==` operator. If we think about conditions under which we would
 -- intuitively -- consider two instances of an optional to be *equal*, we can quickly come up with the following rules.

> Two instances `a` and `b` of an optional are `equal` if and only
> * if both have no values or
> * if both have values, which are `equal`.

We can now derive test cases for these two cases, in which two optionals are equal.

{% highlight cpp %}

TEST_CASE("Two optionals without a value are equal.") {
  optional_unsigned_int x;
  optional_unsigned_int y;
  REQUIRE(x == y);
  REQUIRE(y == x);
}

TEST_CASE("Two optionals with equal values are equal.") {
  unsigned int anyValue = 5;
  optional_unsigned_int x(anyValue);
  optional_unsigned_int y(anyValue);
  REQUIRE(x == y);
  REQUIRE(y == x);
}

{% endhighlight %}

## Equality Comparison as a Member Function

C++ provides multiple mechanisms to implement the `==` operator. One of them is to define this operator as a class
member function. Using this mechanism we can now provide a first implementation to make these two tests pass.

{% highlight cpp %}

class optional_unsigned_int {
 public:
 // ...
 bool operator==(optional_unsigned_int other) {
   return true;
 }
};

{% endhighlight %}

This implementation will obviously make our tests pass but it obviously wrong, too: every two instances of an optional
are equal according to this equality operator definition. We need more test cases.

{% highlight cpp %}

TEST_CASE("An optional without a value and an optional with a value are unequal.") {
  optional_unsigned_int x;
  unsigned int anyValue = 1;
  optional_unsigned_int y(anyValue);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
}

TEST_CASE("Two optionals with unequal values are unequal.") {
  unsigned int anyValue = 5;
  unsigned int anyOtherValue = 6;
  optional_unsigned_int x(anyValue);
  optional_unsigned_int y(anyOtherValue);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
}

{% endhighlight %}

These two test cases are forcing us to provide a proper implementation of the equality operator.

{% highlight cpp %}

class optional_unsigned_int {
 public:
 // ...
 bool operator==(optional_unsigned_int other) {
    if (mHasValue && other.mHasValue) {
        return mValue == other.mValue;
    }
    return !mHasValue && !other.mHasValue;
  }
};

{% endhighlight %}

Now all tests pass again. But this implementation can do even more than just compare two optionals.
We can also compare an optional with another value, which is not an optional -- and it works.
For example we can write this test, which will pass without any further ado.

{% highlight cpp %}
TEST_CASE("An optional with a value and an unsigned value with the same value are equal") {
  unsigned int anyValue = 1;
  optional_unsigned_int x(anyValue);
  REQUIRE(x == anyValue);
}
{% endhighlight %}

This works, because `anyValue` is *implicitly* converted to an optional using the appropriate constructor. An equivalent
implementation would be to convert `anyValue` *explicitly*.

{% highlight cpp %}
REQUIRE(x == optional_unsigned_int(anyValue));
{% endhighlight %}

This is quite nice. But if we swap the variables, it will not compile.

{% highlight cpp %}
unsigned int anyValue = 1;
optional_unsigned_int x(anyValue);
REQUIRE(x == anyValue); // compiles
REQUIRE(anyValue == x); // does not compile
{% endhighlight %}

This is because we defined the `==` operator as a member, so it acts like a member function. We actually can call the
`==` operator like a member function. This notation is actually valid C++!.

{% highlight cpp %}
x.operator==(anyValue); // compiles
anyValue.operator==(x); // does not compile
{% endhighlight %}

Now it should be obvious why the second line does not compile: `anyValue` is of type `unsigned int` which is not a class
type and therefore we can not call functions on it.

One could argue, that this doesn't matter too much. A user should just use the `comparison` operator with operands in
the correct order. But this is undesirable. Users will usually expect that they can exchange the operands as the want.
Equality  comparison should be *commutative*. In order to archive this, we can define the operator as a
*non-member-function* (as recommended by [C.161](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ro-symmetric) of the C++
Core Guidelines). 

## Equality Comparison as a Non-Member Function

First of all we will adapt our new test from above, which tests the comparison of an optional with a non-optional,
so that both cases -- `x == anyValue` and `anyValue == x` are tested.

{% highlight cpp %}
TEST_CASE("An optional with a value and an unsigned value with the same value are equal") {
  unsigned int anyValue = 1;
  optional_unsigned_int x(anyValue);
  REQUIRE(x == anyValue);
  REQUIRE(anyValue == x);
}
{% endhighlight %}

Now our tests are failing and we are forced to adapt our implementation.

{% highlight cpp %}

class optional_unsigned_int {
 public:
 // ...
 friend bool operator ==(optional_unsigned_int a, optional_unsigned_int b) {
   if (a.mHasValue && b.mHasValue) {
       return a.mValue == b.mValue;
   }
   return !a.mHasValue && !b.mHasValue;
 }
};

{% endhighlight %}

Please not the `friend` specifier in the declaration. It is not strictly necessary to declare `==` operator as a friend
function. In our particular case a simple free function would be sufficient, because all necessary information are
accessible using the class' member functions. For our current example, the choice between a friend or a non-friend function
comes down to a matter of taste, but I see the following advantages of the friend function approach (which hold for every
[comparison operator](https://en.cppreference.com/w/cpp/language/operator_comparison)):

1. It will work in every case, even if not all necessary information are accessible via the class' public interface.
   For consistency reasons, it might make sense to always declare comparison operators as friends. This is easily
   enforcable by using static code analysis tools like [SonarQube](https://rules.sonarsource.com/cpp/RSPEC-2807).
2. Even if a comparison operator can be implemented using the class' public interface, it might still be
   desirable to prefer the friend over the non-friend function, because it doesn't put additional requirements on the
   class' public interface: getter-like functions can be dropped without breaking the comparison operator.
3. A friend function can be placed within a class definition just like every other function, which is part of the class'
   public interface. This might help clarity as the class' interface is defined completely in one place. Additionally,
   [C.168](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ro-namespace) is fullfilled without further ado. 
4. In generic code, idioms and implementation techniques like 
   ["Making New Friends"](https://www.youtube.com/watch?v=POa_V15je8Y), the
   ["hidden friends idiom"](https://www.justsoftwaresolutions.co.uk/cplusplus/hidden-friends.html)
   ["Barton–Nackman trick"](https://en.wikipedia.org/wiki/Barton%E2%80%93Nackman_trick) are also relying on friend
   functions, so if might help with out transition to a generic optional.

## A Compatible Inequality Comparison Operator

In C++ the inequality comparison operator `!=` is not automatically generated if the equality comparison operator is
defined. Therefore we need to implement it by ourselfs. At first we will, of course, adapt out tests first. In this
case, it seems reasonable to add just another assertion using the `!=` operator to our tests regarding the `==`
operator. For example, for our first test regarding the equality comparison operator, it looks like this.

{% highlight cpp %}
TEST_CASE("Two optionals without a value are equal.") {
  optional_unsigned_int x;
  optional_unsigned_int y;
  REQUIRE(x == y);
  REQUIRE(y == x);
  REQUIRE(!(x != y));
  REQUIRE(!(y != x));
}
{% endhighlight %}

As our tests are not compiling anymore, we know can add the implementation of the `!=` operator.
We will implement it as a friend function, too. The implementation is actually pretty simple because we can delegate
most of the work to the `==` operator.

{% highlight cpp %}

class optional_unsigned_int {
 public:
 // ...
 friend bool operator !=(optional_unsigned_int a, optional_unsigned_int b) {
   return !(a == b);
 }
};

{% endhighlight %}

## Conclusion

With the implementation of the equality and inequality comparison operators for our optional, we've made a big step in
order to make our optional [regular]({% post_url 2022-01-23-regular-optional %}). We've seen multiple ways
of implementing them and concluded to use the friend-function approach. Well will continue to make our optional a
regular type in our next post by adding the missing comparison operators.
