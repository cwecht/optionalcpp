---
layout: post
title:  "An C++ 98 Optional: Part III - A better solution"
date:   2023-01-29 19:15:00 +0100
---

## Back to the past

Last time we rightly [concluded]({% post_url 2022-10-02-cpp-98-optional-II %}#conclusion) that our current solution
for ensuring that the alignment requirement of an `optional<T>`s value is met for all `T`s is quite a hack and
suboptimal. We can resolve this hack by splitting up the problem of providing correctly aligned storage into two
different problems:

1. Retrieving the alignment of a given type 'T'.
2. Providing correctly aligned storage for a given alignment.

## Retrieve the alignment of `T`

Two of the [alignment rules]({% post_url 2022-10-02-cpp-98-optional-II %}#alignment-rule-wrap-up) can help us here:

1. `struct`s inherit the alignment requirements of their member with the largest alignment
   requirement.
2. For `struct`s the compiler may add *padding bytes* between successive members in order to ensure that their alignment requirements
   of all members are met.

Let's have a look at the following:

{% highlight cpp %}
template <typename T>
struct alignment_probe {
  bool x;
  T probee;
};
{% endhighlight %}


This `struct` will always inherit `T`s alignment requirements *and* will the compiler will make sure that `T` is
properly aligned within the `struct`. The C++ Standard tells us, that a types alignment is the number of bytes,
between successive addresses in memory at which objects of this type may take place. The address of `alignment_prove<T>`
will always be a valid address for `T`, too, as it inherits `T`s alignment requirements. `probee` must be
placed at the *next successive* address at which `T` be allocated in order to ensure that `T`s alignment requirements
are met. The figure below should make this more clear: it shows the placement of `alignment_probe<T>` and it's members
including the additional padding bytes between them.

![probe image]({{site.url}}/images/alignment_probe.svg){: width="740" }

Based on all that we can conclude: that the alignment of `T` is exactly the *number of bytes* between the first byte of
`alignment_probe<T>` and  `alignment_probe<T>::probee`. With the help of the figure above we can also see rather quickly
that this number of bytes is exactly the difference of `sizeof(alignment_probie<T>)` and `sizeof(T)`.

Using this knowledge we can now define a *meta function* which will return the alignment of a given type:

{% highlight cpp %}
template <typename T>
struct alignment_of {
  static const std::size_t value = sizeof(alignment_probe<T>) - sizeof(T);
};
{% endhighlight %}

*Note:* We are entering the field of `template meta programming` (TMP) now. In order to get a bit more familiar with
what TMP (and a meta function in particular) is, please refer to
[this introductory talk by Walter E. Brown](https://www.youtube.com/watch?v=Am2is2QCvxY).

So with `alignemnt_of<T>::value` we have already archived the first step! So let's move on to the second part.

## Provide correctly aligned storage

From the [alignment rules]({% post_url 2022-10-02-cpp-98-optional-II %}#alignment-rule-wrap-up) we know that (in
C++98 and C++03) all possible alignments are provided by build-in types; there is now way in standard C++ to introduce
other alignment requirements. This means that we need to map "alignments" (which is a number of bytes) to build-in
types.

We will now implement

1. implement *meta function* called `type_with_alignment<T>::type` which maps "alignments" to build-in types and
2. implement template class called `aligned_storage<Size, Alignment>`  which provides storage of a given size and
  which fulfills the given alignment requirements.

### Implement `type_with_alignment<T>::type`

For implementing `type_with_alignment<T>`  we will use
[template speciallization](https://en.cppreference.com/w/cpp/language/template_specialization).
We will start with a rather simple class template, which will provide `max_align_t` as the default `type`.

{% highlight cpp %}
template <std::size_t N>
struct type_with_alignment {
  typedef max_align_t type;
};
{% endhighlight %}

While doing that we don't make anything wrong in the sense that `max_align_t` will always provide proper alignment for
any other type; it might only impose to strict alignment requirements on the storage. For smaller requirements than
`sizeof(max_align_t)` we need to provide special cases. Now class template specialization comes in handy.

We will start with the special case for the alignment "4 bytes" which should lead to `int` as the resulting type.

{% highlight cpp %}
template <>
struct type_with_alignment<4> {
  typedef std::int type;
};
{% endhighlight %}

This is actually a good example to showcase what (full) template specialization is about: providing diverging
implementations for specific parameters; in our case for specific values of `N`. We can recognize a (full) template
specialization by `template <>`. The specific parameter (4 in our case) is placed right after the name of the type:
`type_with_alignment<4>`. If now `type_with_alignment<4>` is used anywhere this specialization is picked over the
actual template implementation.

Now we need such specializations for each power of two from 1 to `sizeof(max_align_t)` (remember that only powers of two
are valid values for alignment requirements). For each of these values we need to provide a built-in type which has this
exact alignment.

| Alignment | Built-in Type |
|-----------|---------------|
| 1         | `char`        |
| 2         | `short`       |
| 4         | `int`         |
| 8         | `long`        |

It makes sense to choose the fixed size integer aliases here, because for types such as int or
long, the standard does not define
[their size precisely](https://en.cppreference.com/w/cpp/language/types#Signed_and_unsigned_integer_types).
The implementation of `type_with_alignment<N>` with all of its specializations looks now like this:

{% highlight cpp %}
template <std::size_t N>
struct type_with_alignment {
  typedef max_align_t type;
};

template <>                                          \
struct type_with_alignment<alignment_of<char>::value> {
  typedef char type; 
};

template <>                                          \
struct type_with_alignment<alignment_of<short>::value> {
  typedef short type; 
};

template <>                                          \
struct type_with_alignment<alignment_of<int>::value> {
  typedef int type; 
};

template <>                                          \
struct type_with_alignment<alignment_of<long>::value> {
  typedef long type; 
};
{% endhighlight %}

### Implement `aligned_storage<Size, Alignment>`

`aligned_storage<Size, Alignment>` needs to provide storage of size `Size` *and* with an alignment of `Alignment`.
The storage can be provided by a array of `char`s; the alignment can be provided by adding a member of type
`type_with_alignment<Alignment>::type`. As we *only* need the *size* of the former and the *alignment* of the latter,
it makes sense to put them both into a union.

{% highlight cpp %}
template <std::size_t Size,
          std::size_t Alignment = alignment_of<max_align_t>::value>
struct aligned_storage {
  union type {
    char mStorage[Size];
    typename type_with_alignment<Alignment>::type mAlignmentDummy;
  };
};
{% endhighlight %}

## Fix optional

With this two new facilities we can now fix up our `optional`.

{% highlight cpp %}
template <typename T>
class optional {
 /// ...
 private:
  /// ...
  aligned_storage<sizeof(T), alignment_of<T>::value> mBuffer;

  void constructValue(const T& other) {
    new (&mBuffer.mStorage) T(other);
  }

  void destructValue() {
    reinterpret_cast<T*>(&mBuffer.mStorage)->~T();
  }
};
{% endhighlight %}

## Conclusion

In this post our C++98 optional finally reached "feature parity" with out C++11 base implementation of optional, we had
before. At first we had to find [an alternative to unrestricted unions]({% post_url 2022-09-18-cpp-98-optional-I %});
the we had to fix the deficiencies of this first C++98 based implementation with read to alignment *alignment*, which
took us two post to get -- at least somewhat -- right.

We had to come up with our own facilities to ensure that our optional is properly aligned for every `T`. It turns out
that retrieving the alignment of types and providing aligned storage are both issues which come up quiet regularly. This
is the reason, why both facilities have been present in [boost](https://www.boost.org/) for some time and finally ended
up being part of the standard library in C++11.

| Implementation | retrieve alignment | provide alignment |
|----------------|--------------------|-------------------|
| Our Implementation |`alignment_of<T>::value` | `aligned_storage<S, A>` |
| C++11 library | [std::alignment_of\<T\>::value](https://en.cppreference.com/w/cpp/types/alignment_of) | [std::aligned_storage<S, A>](https://en.cppreference.com/w/cpp/types/aligned_storage) |
| boost.type_traits | [boost::alignment_of\<T\>::value](https://github.com/boostorg/type_traits/blob/develop/include/boost/type_traits/alignment_of.hpp) | [boost::aligned_storage<S, A>](https://github.com/boostorg/type_traits/blob/develop/include/boost/type_traits/aligned_storage.hpp) |
| C++11 language feature | [alignof(T)](https://en.cppreference.com/w/cpp/language/alignof) | [alignas(A) char mChar\[S\]](https://en.cppreference.com/w/cpp/language/alignas) |

A look at the
[implementation of boost::aligment_of](https://github.com/boostorg/type_traits/blob/develop/include/boost/type_traits/alignment_of.hpp)
and
[boost::type_with_alignment](https://github.com/boostorg/type_traits/blob/develop/include/boost/type_traits/type_with_alignment.hppa)
shows us the reason, why language features have been added for both of these cases: both of them have little traps to
consider - our implementation is a bit too simple with this regard. For example there should be special cases for `void`
and pointers/references in the case of `alignment_of`. Also, it seems to be possible, that the size of `alignment_probe`
maybe *any* multiple of the alignment of `T`, which is unfortunate. Boost has a special check for that, but I have
chosen to ignore this case for the sake of simplicity.

But there is more: `alignas` allows to specify arbitrary powers of two as the alignment of a type or expression. Such an
over-aligned type won't with a library-only implementation of `alignment_of`. The language feature `alignof` is needed
here. This is also the reason why `std::alignment_of` is usually implemented in terms of `alignment_of`.

Given these limitations our optional (as it is right now) should be only used in a C++98 environment -- or in a C++11
and above environment very carefully. Note that even if our implementation of `alignment_of` won't determine the correct
alignment of `T`, it *should* always *overestimate* the alignment. The resulting optional will be correctly aligned but
maybe to large. This only applies though for types, which are smaller than `sizeof(max_align_t)`.

Regardless of these remarks: we now can move on with our implementation efforts using C++98 and explore, what we can
archive to implement in this rather old C++ standard - in the upcoming post. 
