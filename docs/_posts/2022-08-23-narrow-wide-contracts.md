---
layout: post
title:  "Wide contracts and narrow contracts"
date:   2022-08-23 20:09:00 +0100
---

## Two interfaces? Two Implementations!

We have now two different interfaces for [getting an optionals value]({% post_url 2022-07-17-pointer-syntax %}):

* the `value()` member function and
* the `*` operator.

Both behaving exactly identical -- having the same *contract*:

> If the optional stores a value, return the value.

A function's or function's (or operator's) *contract* is a specification of it's behavior (please note that this definition of the term
"contract" is rather unspecific, but it will serve our purposes for now). The contract of our accessor function and
operator *only* defines the behavior in case of a value being actually stored in the optional, this function/operator is
called on. The negative case (an optional *without* any value is not covered by this contract: the behavior is
*undefined*.

Such a contract is referred to as a
[narrow contract](https://stackoverflow.com/questions/51292673/what-is-in-simple-understanding-narrow-contract-and-wide-contract-in-terms-of);
a narrow contract defines the behavior of a function/operator only for a specific set of input values. In
contrast to that a *wide contract* defines the behavior for all possible input values.

Both options -- wide or narrow contracts -- are useful depending on the situation at hand. Wide contracts are often
appreciated by the users because they are told exactly what will happen in any case. But there cases, in which not every
invalid input can be dealt with reasonably, maybe because the detection of such invalid input would be to costly or
because if might not even possible (due to issues like the halting problem). For example, let's take for example a
function which takes an `std::vector` of some type an performs a binary search on it.
Such a function will only work if and only if the input values are sorted. An function implementing
binary search could check whether the input values are sorted, but this is only possible with linear complexity in time.
But a binary search with linear complexity has no advantage compared to linear search. In this case, only a narrow
contract will work (note, for this particular example there are other possibilities to archive a wide contract, but for
the sake of this simple example, it doesn't matter). For more background about narrow contracts and undefined behavior,
please refer to [this CppCon talk](https://www.youtube.com/watch?v=yG1OZ69H_-o).

But what can we do now in our particular case? We could check, whether the optional contains a value and throw an
exception otherwise! This is exactly, what the C++ standard specifies for
[value()](https://en.cppreference.com/w/cpp/utility/optional/value). First of all, we need to define an exception type
for this kind of error.

{% highlight cpp %}
class bad_optional_access : public std::exception {};
{% endhighlight %}

Now we can easily create a new test case for accessing the value of an optional without an value:

{% highlight cpp %}
TEST_CASE("value() will throw if it is called on an optional without a value") {
  const optional<int> empty;
  REQUIRE_THROWS_AS(empty.value(), bad_optional_access);
}
{% endhighlight %}

And the corresponding implementation looks like this:

{% highlight cpp %}
template <typename T>
class optional {
 public:
  const T& value() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
    return mValue;
  }
};
{% endhighlight %}

`optional::value()` has now a wide contract which can be phrased like this:

> If the optional stores a value, return the value.
> If the optional stores no value, throw a `bad_optional_access` exception.

Should we adapt this wide contract now for the `*`-operator, too? Probably not, because of multiple reasons:
  * exceptions are one of the most controversial topics in C++. In many projects, the use of exceptions are
  [not allowed](https://google.github.io/styleguide/cppguide.html#Exceptions). In (deeply) embedded projects they are
  quite often disabled entirely, because exceptions are usually implemented in a way which will significantly increase
  the binary size (and flash memory is usually scarce on small plattforms. Exceptions do not follow the
  [zero-overhead principle](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle). These are not all issues, which can be raised
  about C++'s exceptions; for us it is important to be aware of the fact, that exceptions may not be the mechanism of
  choice for many people.
  * Dereferencing  a null pointer (regardless whether we are talking about raw pointer or smart pointer) using the
    `*`-operator is undefined behaviour. As we designed to pointer-style-accessors to
    [mimic the syntax of pointers]({% post_url 2022-07-17-pointer-syntax %}), it makes (at least some) sense to mimic the behavior as
    well.
  * `std::vector` has also narrow (`std::vector::operator[]`) and wide contract (`std::vector::at()`) interfaces.

Because of these reasons -- and because the C++ standard decided so as well -- we will keep the narrow contract of
`optional::operator*`.

## Conclusion

In this post, we learned distinguish *narrow contracts* and *wide contracts*. We saw, that both of them have there uses
and opted for using both "contract styles" as we have already two different interfaces for the same task (accessing the
value of an optional).

Fortunately the accessor member functions and operators (`value()`, `*` and `->`) are the only members with a narrow contract for
now -- every other member function or operator has a wide contract already, so there is nothing to worry about here.
