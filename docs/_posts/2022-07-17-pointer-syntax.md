---
layout: post
title:  "We always had an optional! (kind of...)"
date:   2022-07-17 18:13:00 +0100
---

## Widening our view

Since at least [our last post]({% post_url 2022-06-26-copy-assignment-revisited %}) we've implemented a pretty decent optional!
But what if I told you, that even C had already some kind of optional? But let's take a step back! What makes an
optional an optional?

1. It *either* contains a value *or* nothing.
2. We can query an optional whether it contains a value or not.
3. We can retrieve the value if there is any.

Turns out: a ordinary pointer just like in C has exactly these properties:

1. A pointer *either* points to an value *or* is the null pointer.
2. We can query a pointer either
  * by comparing it to the null pointer or
  * by converting it to a boolean.
3. We can retrieve the value by dereferencing the pointer (if it is not the null pointer).

Using a pointer as some kind of optional is actually quite common. For example
[std::stoi](https://en.cppreference.com/w/cpp/string/basic_string/stol) is declared like this:

{% highlight cpp %}
int stoi( const std::string& str, std::size_t* pos = nullptr, int base = 10 );
{% endhighlight %}

Please note, that `pos` is a pointer, which is defaulted to the null pointer. `pos` is in fact an optional *out
parameter*: it well store the number of characters, it processed in the value the pointer points to, if there is any.
In case, you as an caller of this function don't want to use this feature, you can just pass an null pointer and
everything will work out.

[std::fopen](https://en.cppreference.com/w/cpp/io/c/fopen) does something similar: it uses a pointer as it's return
type:

{% highlight cpp %}
std::FILE* fopen( const char* filename, const char* mode );
{% endhighlight %}

`std::fopen` will return the null pointer if it could not open the file. This makes totally sense: if the file could not
be opened, what else should `std::fopen` return than *nothing*?

A third quite popular use case for optional-like pointers is
[lazy initilialization of class members](https://www.cppstories.com/2019/10/lazyinit/). Let's assume, that we wanted to
implement a class with a member, which could not be initialized reasonably in the constructor; maybe we need to do the
initialization later on (maybe this doesn't happen to often). Especially in older (pre C++11) code bases, you might find
a solution for this problem which basically looks like this:

{% highlight cpp %}
class ClassWithLazyInitializedMember {
  public:
    ClassWithLazyInitializedMember()
      : mLazyMember(NULL) {}

    ~ClassWithLazyInitializedMember() {
      delete mLazyMember;
    }

    void initialize() {
       mLazyMember = new LazyObject();
    }
  private:
    LazyObject mLazyMember;
};
{% endhighlight %}

In newer code bases (C++11 and later) this solution might be implemented using a
[std::unique_ptr](https://en.cppreference.com/w/cpp/memory/unique_ptr), but the basic idea is the same:
as we can not initialize `LazyObject` in the constructor, we only initialize a *pointer* to `LazyObject` there.
The initialization of `LazyObject` is done later on in the `initialize()` member function.

This solution will work but it has some issues:
1. It uses the heap no matter if it make sense or not, which is quite wasteful.
2. `ClassWithLazyInitializedMember` can not be copied anymore. In the solution above, this is because we had to provide
   a user defined destructor. If we'd use `std::unique_ptr`, it couldn't be copied, because `std::unique_ptr` can not be
   copied. (A `std::shared_ptr` could be copied, but that would be only a *shallow copy*). In each of these cases we
   would need to implement the copy operations in order to make the class  [regular]({% post_url 2022-01-23-regular-optional %}).

In such a case using a optional would often be a better solution.

{% highlight cpp %}
class ClassWithLazyInitializedMember {
  public:
    void initialize() {
       mLazyMember = LazyObject();
    }
  private:
    optional<LazyObject> mLazyMember;
};
{% endhighlight %}

By using optional we can archive basically the same behavior as with the pointer based solution. But additionally
1. We don't need to use the heap anymore.
2. `ClassWithLazyInitializedMember` is now *regular* because `optional<LazyObject>` is *regular* if `LazyObject` is
   regular.

These three examples (hopefully) could illustrate, how pointers are used as optional-like types. This holds true not
only for raw pointers (like `int *x`) but also for *smart pointers* like `std::unique_ptr<int>` or
`std::shared_ptr<int>`. As of now they have at least one advantage over our optional: their terse syntax.

|operation              | `optional<AnyStruct>` | `AnyStruct*` |
|-----------------------|-----------------------|--------------|
| Check for value       | `x.value()`           | `*x`         |
| access value          | `if (x.has_value())`  | `if (x)`     |
| access member of value| `x.value().f()`       | `x->f()`     |

It would be nice though, if these "optional-like types" would have a common syntax for these operations. It would also
make our optional a much better drop-in replacement for pointers. Thankfully C++ offers the appropriate facilities for
our optional to adopt this syntax.

## Test the new Syntax

Implementing the required tests for the new syntax is really straight forward: we can basically reuse existing test
cases and replace `value()` and `has_value()` with the new syntax.

{% highlight cpp %}
TEST_CASE("An default constructed optional converts to 'false'.") {
  const optional_unsigned_int x;
  REQUIRE(!x);
}

TEST_CASE("An optional constructed with a value converts to 'true'.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(x);
}

TEST_CASE("An optional constructed with a value can be dereferenced.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(*x == anyValue);
}

TEST_CASE("An optional constructed with a value can access members directly.") {
  struct A {
    unsigned int x;
  };
  unsigned int anyValue = 10;
  A anyStructValue = {anyValue};
  const optional<A> a(anyStructValue);
  REQUIRE(a->x == anyValue);
}

{% endhighlight %}

Only for the `->` operator we need to introduce a `struct` (or a `class`) with a member, we can acces in the test.

## Implementation

The actuall implementation is equally straigh forward as our new "methods" are basically simple getter functions. The
only tricky part is in the syntax for overloading these operators.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  explicit operator bool() const {
    return mHasValue;
  }

  const T& operator*() const {
    return mValue;
  }

  const T* operator->() const {
    return &mValue;
  }
  //...
};

{% endhighlight %}

There are a few things to note here:
1. Overloading the dereferencing operator (`*x`) is actually not that hard: the implementation is exactly the same as
   of `value()` -- only `value`  has been replaced with `operator*`.
2. The return value of the `->` operator has the restriction, that itself must support the `->` as well. Using a pointer
   does exactly that by remaining the closest fit to a reference.
3. The syntax of the "conversion to bool" operator is a bit odd: in order to avoid ambiguity with
   [function call operator](https://en.cppreference.com/w/cpp/language/operators#Function_call_operator) it's return 
   value is written *after* the operator keyword (instead of before it like usually). We additionally needed to make
   the conversion explicit in order to avoid unintended conversion to integers. C++'s implicit conversion rules are
   quite complicated. For now it is sufficient for us to know that:
   * the 'explicit' keyword prohibits implicit conversion to integers. `int y = optional<int>{};` will not compile.
     Without the `explicit` this expression would compile.
   * `bool y = optional<int>{}` won't compile either, but this should be fine, as such a line of code would be quite odd
     anyways and quite hard to read.
   * Expressions like `if (optional<int>{})' will work. An if expression oviously counts as an explicit conversion to
     `bool`.
   * [std::optional::operator bool](https://en.cppreference.com/w/cpp/utility/optional/operator_bool) is marked
     `explicit` as well.

## Conclusion

In this blog post we saw, that pointers and optionals are quite similar. In order to align the interfaces, we overloaded
the "coversion to bool" operator, the dereferencing operator and the `->` operator.

We will explore the similarities an differences between pointers and optionals even further in an upcoming post.
