---
layout: post
title:  "The return of assignment"
date:   2022-06-18 19:33:00 +0100
---

## Getting back to regular

Since we had to implement our own [destructor]({% post_url 2022-05-28-a-new-destructor %}) and
[copy constructor]({% post_url 2022-05-29-copy-constructor-strikes-back %}) (if we want to be able to assign to
optionals with a non trivial value type), we have to assume, that we need to implement our own
copy assignment operator as well. After that our optional we be [regular]({% post_url 2022-01-23-regular-optional %})
again.

As always, our first step must be to add failing tests. As our existing test cases regarding copy assignment are
residing in the "An optional and its copy are equal." section of our test suite, we will put these new test cases in
there as well.

{% highlight cpp %}
TEST_CASE("An optional and its copy are equal.") {
  // ...
  SECTION("copy assignment without a value (non trivial)") {
    optional<std::vector<int>> x;
    optional<std::vector<int>> y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assignment with a value (non trivial)") {
    std::vector<int> anyValueX = {1, 2, 3};
    optional<std::vector<int>> x(anyValueX);
    optional<std::vector<int>> y;
    y = x;
    REQUIRE(x == y);
  }
  // ...
}
{% endhighlight %}

These tests won't -- of cause -- compile: the compiler couldn't synthesis a
[copy constructor]({% post_url 2022-05-29-copy-constructor-strikes-back %}) for `std::string`; there is no reason
to believe, that it could do it for the copy assignment operator  -- actually it can't for the same reason: the `union`.

As soon as we add any syntactically correct implementation of a copy assignment operator, this tests will compile again.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  /// ...
  void operator=(const optional& other) {}
  /// ...
};
{% endhighlight %}

Note, that this signature of an assignment operator is quite unusual: we'll get back to that in the upcoming post.
With this implementation, the tests are compiling again, but are failing now, but it is a good starting point for the
actual implementation.

## A first naive approach.

A first naive approach could be to take the [copy constructor]({% post_url 2022-05-29-copy-constructor-strikes-back %})
as an example, only we would only need to make a few adaptions:

* As the assignment operator is not a constructor, it has no initializer list. Therefore we need to assign
  `other.mHasValue` to `mHasValue`.
* We should assign `other.mValue` to `mValue` is `other.mHasValue` is true.

Such an implementation would look like this.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  /// ...
  void operator=(const optional& other) {
    mHasValue = other.mHasValue;
    if (other.mHasValue) {
      mValue = other.mValue;
    }
  }
  /// ...
};
{% endhighlight %}

This implementation will compile, but it will raise a SEGFAULT for the "copy assignment with a value (non trivial)"
 case. The reason for this is, that the assignment operator is basically a member function, an `mValue` is in this case
no valid object. The situation is this:

1. If (like in our test case) the target optional of the assignment has now value.
2. This means, that `mValue` does not store any valid value. This means, that we can not call any member functions on
   this object.
3. The assignment operator is a member function. Actually, we can transform an expression like `a = b;` equivalently to
   `a.operator=(b)`.
4. This implies, that you can not assign to an invalid object.
5. But this is the case in the "copy assignment with a value (non trivial)" test case.

From this issue, we should draw these two conclusions:
1. The assignment to an optional without a value must be treated differently. 
2. We need more test cases.

## A few more test cases

Obviously, we need to test these four cases:

| Target | Source | Test Case Name |
---------|--------|----------------|
optional without a value | optional without a value | "copy assign optional without a value to an optional without a value" |
optional without value | optional with a value | "copy assign optional with a value to an optional without a value" |
optional with a value | optional without value | "copy assign optional without a value to an optional with a value" |
optional with a value | optional with a value | "copy assign optional with a value to an optional with a value" |

The implementations could look like this:

{% highlight cpp %}
/// ...
TEST_CASE("An optional and its copy are equal.") {
  ///...
  SECTION(
      "copy assign optional without a value to an optional without a value") {
    optional<std::vector<int>> x;
    optional<std::vector<int>> y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional with a value to an optional without a value") {
    std::vector<int> anyValueX = {1, 2, 3};
    optional<std::vector<int>> x(anyValueX);
    optional<std::vector<int>> y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional without a value to an optional with a value") {
    optional<std::vector<int>> x;
    std::vector<int> anyValueY = {1, 2, 3};
    optional<std::vector<int>> y(anyValueY);
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional with a value to an optional with a value") {
    std::vector<int> anyValueX = {1, 2, 3};
    optional<std::vector<int>> x(anyValueX);
    std::vector<int> anyValueY = {1, 3};
    optional<std::vector<int>> y(anyValueY);
    y = x;
    REQUIRE(x == y);
  }
}
/// ...
{% endhighlight %}

Note, that, if we exclude the test case, of which we know, that it will crash, all other tests are passing now. This
gives us a strong indication, that our first naive approach was not that bad after all. We can build upon it.

## A better solution

{% highlight cpp %}
template <typename T>
class optional {
 public:
  /// ...
  void operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      mValue = other.mValue;
    } else if (other.mHasValue) {
      new (&mValue) T(other.mValue);
    }
    mHasValue = other.mHasValue;
  }
  /// ...
};
{% endhighlight %}

With this version, all our test cases will pass. Please note, that only two cases which we listed above, are covered
explicitly:

* The "copy assign optional with a value to an optional with a value" case is covered by the first branch of the if-else
  statement. In this case, we can safely rely on `T`'s assignment operator to work properly. 
* The "copy assign optional with a value to an optional without a value" case is covered by the second branch. In this
  case, we need to construct a copy of `other`'s value instead of assigning it to an existing value.
* In the "copy assign optional without a value to an optional without a value" there is no value involved, so `mValue`
  can be ignored. 
* The "copy assign optional without a value to an optional with a value" case is not covered at all. This should raise
  suspicion, because the lifetime of the value of the target optional ends with this assignment. Usually at the end of
  the lifetime of an object, it's destructor must be called. In our test case, this is quite important because a
  non-empty vector is used, which uses heap allocated memory. This memory is usually freed within the destructor, but if
  the destructor is not executed, this [memory leaks](https://en.wikipedia.org/wiki/Memory_leak).

So, our current implementation has a potential memory leak. As memory leaks don't cause crashed (usually) we were not
able to observe it yet. Only if we would perform lot's of assignments, we would observe, that the memory consumption of
the program would increase (it is actually a quite common way of detection memory leaks in larger projects).

## Diagnosing memory leaks using valgrind

But there are tools, which can help us to diagnose memory leaks. One of the is [valgrind](https://valgrind.org/).
Please note, that valgrind is Linux tools. For Windows there is [Visual Studio's
Deleaker](https://www.deleaker.com/blog/2020/01/04/valgrind-for-windows/) which does similar things.

{% highlight bash %}
valgrind --leak-check=full ./test_optionalcpp
{% endhighlight %}

If we execute out test executable like this, we get an output like this.

{% highlight bash %}
==51050== 
==51050== HEAP SUMMARY:
==51050==     in use at exit: 12 bytes in 1 blocks
==51050==   total heap usage: 1,389 allocs, 1,388 frees, 175,815 bytes allocated
==51050== 
==51050== 12 bytes in 1 blocks are definitely lost in loss record 1 of 1
==51050==    at 0x483BE63: operator new(unsigned long) (in /usr/lib/x86_64-linux-gnu/valgrind/vgpreload_memcheck-amd64-linux.so)
==51050==    by 0x218C6F: __gnu_cxx::new_allocator<int>::allocate(unsigned long, void const*) (new_allocator.h:114)
==51050==    by 0x214AA2: std::allocator_traits<std::allocator<int> >::allocate(std::allocator<int>&, unsigned long) (alloc_traits.h:443)
==51050==    by 0x20E94B: std::_Vector_base<int, std::allocator<int> >::_M_allocate(unsigned long) (stl_vector.h:343)
==51050==    by 0x2148DE: std::_Vector_base<int, std::allocator<int> >::_M_create_storage(unsigned long) (stl_vector.h:358)
==51050==    by 0x20E69A: std::_Vector_base<int, std::allocator<int> >::_Vector_base(unsigned long, std::allocator<int> const&) (stl_vector.h:302)
==51050==    by 0x2019FE: std::vector<int, std::allocator<int> >::vector(std::vector<int, std::allocator<int> > const&) (stl_vector.h:552)
==51050==    by 0x1F466F: optional<std::vector<int, std::allocator<int> > >::optional(std::vector<int, std::allocator<int> > const&) (optional.hpp:13)
==51050==    by 0x1C6B0A: ____C_A_T_C_H____T_E_S_T____14() (tests.cpp:168)
==51050==    by 0x1D967F: Catch::FreeFunctionTestCase::invoke() const (catch_test_case_registry_impl.hpp:150)
==51050==    by 0x1B92D0: Catch::TestCase::invoke() const (catch_test_case_info.hpp:176)
==51050==    by 0x1D816D: Catch::RunContext::invokeActiveTestCase() (catch_run_context.hpp:367)
==51050== 
==51050== LEAK SUMMARY:
==51050==    definitely lost: 12 bytes in 1 blocks
==51050==    indirectly lost: 0 bytes in 0 blocks
==51050==      possibly lost: 0 bytes in 0 blocks
==51050==    still reachable: 0 bytes in 0 blocks
==51050==         suppressed: 0 bytes in 0 blocks
==51050== 
==51050== For lists of detected and suppressed errors, rerun with: -s
==51050== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
{% endhighlight %}

Valgrind provides us quite some information here:

1. As part of the `HEAP SUMMARY` valgrind tells us the number of allocations ("allocs") and deallocations
   ("deallocations). In our case, we have 1389 allocations vs 1388 deallocations, so there is one deallocation missing.
   This is the proof, that we actually leaks memory.
2. After that, we see the [stack trace](https://en.wikipedia.org/wiki/Stack_trace) of the point in the code, at which
   the leaks memory has been allocated. We can see here, that the leaked memory has bee allocated in the optionals copy
   constructor (in `optional.hpp` in line 13). Also we can see the line in the `tests.cpp` where it happened: 168, the
   construction of `y`.
3. Finally there is the `LEAK SUMMAR`, which tells us, that we leaked 12 bytes.

Now, we don't only know, that there is a memory leak in our tests; we also can now prove our fix.

## A better solution without a memory leak

So what do we need to do? We need to destroy the value stored in the target object by calling it's destructor (if we
assign an optional without a value to it).

{% highlight cpp %}
template <typename T>
class optional {
 public:
  /// ...
  void operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      mValue = other.mValue;
    } else if (other.mHasValue) {
      new (&mValue) T(other.mValue);
    } else if (other.mHasValue) {
      mValue.~T();
    }
    mHasValue = other.mHasValue;
  }
  /// ...
};
{% endhighlight %}

The tests are still passing, but even, if we run the tests with valgrind, we get an output like this:

{% highlight bash %}
==51926==
==51926== HEAP SUMMARY:
==51926==     in use at exit: 0 bytes in 0 blocks
==51926==   total heap usage: 1,389 allocs, 1,389 frees, 175,815 bytes allocated
==51926==
==51926== All heap blocks were freed -- no leaks are possible
==51926==
==51926== For lists of detected and suppressed errors, rerun with: -s
==51926== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
{% endhighlight %}

And we see: the numbers of `allocs` and `frees` are matching now! valgrind even tells us explicitly, that there are not
leaks anymore.

Please note, that this implementation circumvents an typical issue of assignment operators: self assignment (see also 
[C.62](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c62-make-copy-assignment-safe-for-self-assignment).
There are only two cases:
1. the optional has no value: in this case, `mHasValue` is assigned to itself, which is fine, since it is initialized
   properly.
2. the optional has a value: in the case, `mHasValue` is assigned to itself, too. Additionally, `mValue` is assigned to
   itself. As we are using `T`'s copy assignment operator here, we can delegate the responsibility to cope with the
   self-assignment. `T`'s copy assignment operator has to make sure, that this case is handled properly now.

## Conclusion

In this post, we managed to implement a new assignment operator for our optional. It was a bit harder than implementing
the copy constructor, but this is kind of expected. Especially if one want's to provide a
[strong exception safety](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Copy-and-swap) for a calls, things may get
complicated. This lead to the wide adoption of the
[Copy-Swap idiom](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Copy-and-swap). We basically ignored this issue.
Actually, we rely on the exception safety guarantees of `T`, because we delegate this responsibility to `T`'s
assignment operator. This is actually exactly the same behavior of
[std::optional](https://en.cppreference.com/w/cpp/utility/optional/operator%3D).

Note, that our optional is now [regular]({% post_url 2022-01-23-regular-optional %}) again. But we are still not done
with the assignment operator: there is detailed left open, which we should cover: in the upcoming post.
