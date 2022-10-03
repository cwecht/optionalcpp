---
layout: post
title:  "An C++ 98 Optional: Part II - A simple hack"
date:   2022-10-03 21:00:00 +0100
---

## Back to the past

After finishing [the ground work]({% post_url 2022-09-18-cpp-98-optional-I %}) we can now focus on the crucial parts, we
need to change in order to become compliant with C++98: we must replace the
([unrestricted](https://www.programmerall.com/article/940842653/)) `union` with something else. Therefore we
need to touch ever line of code which takes advantage of the `union`.

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

But how can we get rid of the `union`?. We needed to union in the first place to avoid the construction of `T` in the
case of an `optional` without a value and to enable the use of types
[without a default destructor]({% post_url 2022-05-27-enable-non-default-constructed-types %}). So the basic issue was,
that we needed more fine grained control of the lifetime of `mValue` -- the stored value.
The [lifetime of any object](https://en.cppreference.com/w/cpp/language/lifetime) begins basically after initialization
has been completed (e.g. by calling the constructor) and as soon as the destructor is called. This means that we can
control an objects lifetime by calling constructor and destructor. We can see above, that we are basically doing this
already: we call the constructor using *placement new operator* in `constructValue` and the *destructor* in
`destructValue`. In the `constructValue` function `mValue` serves as storage to the 
[placement new operator](https://en.cppreference.com/w/cpp/memory/new/operator_new) which then will create the object
at this storage location.

The placement new operator is actually not limited to unions as storage providers, we can basically pass any (almost --
we'll get back to that) memory location , which provides enough memory to store a object of the desired type in it, to
the placement new operator. This includes e.g. an array of `char`s. This comes in quite handy, as a `char` is exactly
one byte large, meaning, that we can easily create an array of `char`s which is exactly as big as we need it to be.

{% highlight cpp %}
char mBuffer[sizeof(T)];
{% endhighlight %}

Just in case you didn't know it already: `sizeof` returns the size of the given type in bytes.

Therefore we can replace the union with `mBuffer` as a first step. Now we just need a way to access the object, which is
stored in the memory provided by `mBuffer`. This can be done with
[reinterpret_cast](https://en.cppreference.com/w/cpp/language/reinterpret_cast). Beside other things, `reinterpret_cast`
can be used to convert a pointer to any other pointer type; hence we can use it to convert `&mBuffer` to `T*`, just like
this.

{% highlight cpp %}
char mBuffer[sizeof(T)];
new(&mBuffer) T();
T* ptr = retinterpret_cast<T*>(&mBuffer);
{% endhighlight %}

With this tools at hand, we now can adapt our optional -- anticipating the transition to C++98 -- by
1. Replacing `mValue` and the `union` definition with the array of`char`s `mBuffer` and
2. Migrating each access to `T` to the respective `reinterpret_cast`.

{% highlight cpp %}
template <typename T>
class optional {
 public:
  //...
  const T& operator*() const {
    return *reinterpret_cast<const T*>(&mBuffer);
  }

  T& operator*() {
    return *reinterpret_cast<T*>(&mBuffer);
  }
  //...

 private:
  //...
  char mBuffer[sizeof(T)];
  //...

  void constructValue(const T& other) {
    new (&mBuffer) T(other);
  }

  void destructValue() {
    reinterpret_cast<T*>(&mBuffer)->~T();
  }
};
{% endhighlight %}

Note, that we had to cast `&mBuffer` to `const T*` in the `const`-version of the `*`-operator. This is necessary because
`reinterpret_cast` can not "cast away" the `const`; in this version of the `*`-operator `mBuffer` -- and therefore
`&mBuffer` -- is `const`, so we can only cast it to `const T*`. This is one of the advantages of C++-style casts over
[c-style casts](https://en.cppreference.com/w/c/language/cast): they circumvent only parts of the type system, not all of
it.

With these changes, we now can go back to C++98 by adapting `CMAKE_CXX_STANDARD` in the `CMakeLists.txt` accordingly.

{% highlight cmake %}
set(CMAKE_CXX_STANDARD 98)
{% endhighlight %}

Our tests will compile and succeed. Are we already done?

Actually we aren't. Our current implementation relies on
[undefined behaviour](https://en.wikipedia.org/wiki/Undefined_behavior). And the underlying issue is all about *alignment*.

## Data alignment

What exactly is *alignment*? The C++ says: 

> Object types have alignment requirements which place restrictions on the addresses at which an
object of that type may be allocated. An alignment is an implementation-defined integer value representing
the number of bytes between successive addresses at which a given object can be allocated. An object type
imposes an alignment requirement on every object of that type.

In C++ objects can not be placed anywhere in memory on any arbitrary address; each type can only be placed at certain
addresses in memory which are defined by the *alignment requirements* of the respective types. The Standard defines the
*alignment of a type* as the "number of bytes between successive addresses at which a given object can be allocated".
This *usually* means (at least on most modern platforms) that a object can only be placed at addresses which are an
integer multiple of its *alignment*.

"Ok, but why is there such a thing as *alignment* at all?", you may ask. Well,
[there are several reasons](https://stackoverflow.com/questions/381244/purpose-of-memory-alignment#381368), but for us
the most important may be, that there are platforms which can
[access values in memory faster](https://learn.microsoft.com/en-us/cpp/c-language/alignment-c?view=msvc-170) if they are
*aligned* (they are placed at a memory locations which is a multiple of their alignment). On
[x86](https://en.wikipedia.org/wiki/X86) this might (at least on modern CPU) not be an issue, but on some ARM platform,
it will. Actually, there are some instructions on x86, which rely on the alignment of their operands, so alignment might
even be an issue in x86. We'll get back to that.

Alignment requirements are important for build-in types such as `int` or `float`. Processors have special instructions
to deal with values of these types efficiently. These operations often require their operands to be aligned. The
alignment of build-in types is identical with their size, meaning that e.g. a `float` (`sizeof(float)` is 4) must be *4
bytes aligned*.

The compiler must ensure that all build-in types
([fundamental types](https://en.cppreference.com/w/cpp/language/types) as well as
[pointer types](https://en.cppreference.com/w/cpp/language/pointer) and
[references](https://en.cppreference.com/w/cpp/language/reference)) are placed at locations matching the respective 
alignment requirements, if they are placed in static memory (basically global variables of all kinds) or on the stack.
Note the alignment of dynamic memory can not be controlled by the compiler as dynamic allocation is done by library
functions like [malloc](https://en.cppreference.com/w/c/memory/malloc) or the new-operator; these functions must provide
properly aligned memory (we'll get back to that).

But there are not only build-in types, there are also
"composed types" (not a standard's term!) which are *composed* out of other types. These
types are `struct`s,`class`es, `union`s and arrays. For these types, the compiler must make sure, that each of it's
members is *always* (under all circumstances) aligned properly. In case of arrays this can be easily achieved by
imposing the exact same alignment requirements on the array as on it's elements.

For `union`s this is not that easy: a `union` may have members with differing alignment requirements. At this point, a
particular property of all alignment requirements comes in handy: all alignment requirements are a power of two. This
implies, that given two alignment requirements the larger one of them will *always* be an integer multiple of the smaller
one. If an address is aligned according to the larger alignment requirement, it will *also* be aligned according to
smaller alignment requirements. For `union`s this means, that they will be aligned properly for all their members if the
`union` inherits the alignment requirements of the member with the largest alignment requirement.

`struct`s and `class`es need an additional technique to ensure the alignment requirements of their members. We need to
take into account here that the order in which the members of a `class` in memory must be exactly the same as in it's
definition, so there is now room for the compiler to reorder them. But it may add *padding bytes*. The compiler is
allowed to place "unused bytes" between the members of a `class` in order to ensure the alignment requirements of each
member (An instructive example can he found
[here](https://stackoverflow.com/questions/381244/purpose-of-memory-alignment#381368). 

Note, that the addition of padding bytes will (obviously) increase the size of the `class`. In many cases the size of a
`class` will be larger than the sum of it's members.

## Alignment Rule Wrap Up

Let's wrap up the rules of alignment in C++98.

1. Every object (everything in memory) has *alignment requirements*, which are integer values and a power of two.
2. "Build-in types" such as [fundamental types](https://en.cppreference.com/w/cpp/language/types) (except `void`),
   all pointer and reference types are aligned by their size.
3. Arrays inherit their alignment requirements form their elements.
4. `struct`s, `class`es and `union`s inherit the alignment requirements of their member with the largest alignment
   requirement.
5. For `struct`s/`class`s the compiler may add *padding bytes* between successive members in order to ensure that their alignment requirements
   of all members are met.
6. By applying these rules the alignment of each type compliant with the C++ Standard can be determined (in C++98).

Note that these rules imply, that there is a maximum alignment requirement defined by the largest "build-in type".
This is true only for C++98 and C++03 though.
C++11 added a specific keyword [alignas](https://en.cppreference.com/w/cpp/language/alignas) with which the
alignment of a type or variable can be specified explicitly. With this keyword larger alignments are possible.
This is referred to as
[overalignment](https://stackoverflow.com/questions/8732441/what-is-overalignment-of-execution-regions-and-input-sections).

Please note, that I *do not claim to be an expert on alignment*. If someone finds an issue with these rules, please let
me know, so that I can fix this post.

## Alignment by Example

Our optional can provide us here a nice example. Let's go back to
our starting point. 

{% highlight cpp %}
#include <iostream>

struct optional_unsigned_int {
    bool mIsSet;
    unsigned int mValue;
};

int main() {
    std::cout << "sizeof(optional_unsigned_int) == "
              << sizeof(optional_unsigned_int) << std::endl;
    std::cout << "sizeof(bool) + sizeof(unsigned int) == "
              << sizeof(bool) + sizeof(unsigned int) << std::endl;
}
{% endhighlight cpp %}

If we [execute this
code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:2,endLineNumber:13,positionColumn:2,positionLineNumber:13,selectionStartColumn:2,selectionStartLineNumber:13,startColumn:2,startLineNumber:13),source:'%23include+%3Ciostream%3E%0A%0Astruct+optional_unsigned_int+%7B%0A++++bool+mIsSet%3B%0A++++unsigned+int+mValue%3B%0A%7D%3B%0A%0Aint+main()+%7B%0A++++std::cout+%3C%3C+%22sizeof(optional_unsigned_int)+%3D%3D+%22%0A++++++++++++++%3C%3C+sizeof(optional_unsigned_int)+%3C%3C+std::endl%3B%0A++++std::cout+%3C%3C+%22sizeof(bool)+%2B+sizeof(unsigned+int)+%3D%3D+%22%0A++++++++++++++%3C%3C+sizeof(bool)+%2B+sizeof(unsigned+int)+%3C%3C+std::endl%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:51.63049676963605,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:executor,i:(argsPanelShown:'1',compilationPanelShown:'0',compiler:g122,compilerOutShown:'0',execArgs:'',execStdin:'',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'',source:1,stdinPanelShown:'1',tree:'1',wrap:'1'),l:'5',n:'0',o:'Executor+x86-64+gcc+12.2+(C%2B%2B,+Editor+%231)',t:'0')),k:48.36950323036395,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4), we'll see, that the size of `optional_unsigned_int` is 8 while the sum of it's member's sizes is 5. The reason for that is, that the compiler added 3 *padding bytes* between `mIsSet` and `mValue` in order to make sure, that `mValue` is always 4 byte aligned.

Now let's make the same test with an memory layout equivalent `struct` to our modified optional implementation.

{% highlight cpp %}
#include <iostream>

struct optional_unsigned_int {
    bool mIsSet;
    char mValue[sizeof(unsigned int)];
};

int main() {
    std::cout << "sizeof(optional_unsigned_int) == "
              << sizeof(optional_unsigned_int) << std::endl;
    std::cout << "sizeof(bool) + sizeof(unsigned int) == "
              << sizeof(bool) + sizeof(unsigned int) << std::endl;
}
{% endhighlight cpp %}

If we now [execute this
code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:2,endLineNumber:13,positionColumn:2,positionLineNumber:13,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:'%23include+%3Ciostream%3E%0A%0Astruct+optional_unsigned_int+%7B%0A++++bool+mIsSet%3B%0A++++char+mValue%5Bsizeof(unsigned+int)%5D%3B%0A%7D%3B%0A%0Aint+main()+%7B%0A++++std::cout+%3C%3C+%22sizeof(optional_unsigned_int)+%3D%3D+%22%0A++++++++++++++%3C%3C+sizeof(optional_unsigned_int)+%3C%3C+std::endl%3B%0A++++std::cout+%3C%3C+%22sizeof(bool)+%2B+sizeof(unsigned+int)+%3D%3D+%22%0A++++++++++++++%3C%3C+sizeof(bool)+%2B+sizeof(unsigned+int)+%3C%3C+std::endl%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:51.63049676963605,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:executor,i:(argsPanelShown:'1',compilationPanelShown:'0',compiler:g122,compilerOutShown:'0',execArgs:'',execStdin:'',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'',source:1,stdinPanelShown:'1',tree:'1',wrap:'1'),l:'5',n:'0',o:'Executor+x86-64+gcc+12.2+(C%2B%2B,+Editor+%231)',t:'0')),k:48.36950323036395,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4), we'll see that *now* the size of `optional_unsigned_int` is 5. The compiler didn't add any padding bytes. The reason for that is, that `mValue` now is an array of `char`s. `char`s only need to be 1 byte aligned. As arrays inherit the alignment requirements of their elements, the array also only needs to be 1 byte aligned. A 1 byte aligned object can be placed anywhere in memory, so there is no need to add additional padding bytes here.

## The issues with our naive implementation

The example above pretty much shows already, what the issue with our new C++98 implementation may be: in almost all
cases `mBuffer` will not be aligned properly, as it is just an array of `char`s. Even for types such as `float`
and `int` it will not match their alignment requirements. It is *undefined behaviour* to
[supplying an unaligned address to placement-new](https://stackoverflow.com/a/39908990), but we just did it.
The only reason why our tests didn't fail is that we ran the tests on a x84 machine (at least I did so), which can deal with
unaligned memory access, and a GCC version, which generated "working code" for our tests. As we invoked undefined
behavior here, the compiler wasn't even required to generated any code here. It just did it more or less out of mere luck.

Was there any chance we could have encountered the issue at some point in time? Actually there is a way we could have
detected the issue with our current test suite: by using sanitizers. GCC's
[undefined behavior sanitizer](https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan)
can detect several cases of undefined behavior during runtime by augmenting the binary with additional checks -- and one of these checks
is about alignment issues. We can enable the undefined behavior sanitizer we need to rerun cmake like this:

{% highlight bash %}
cmake .. -DCMAKE_CXX_FLAGS=-fsanitize=undefined
{% endhighlight bash %}

After recompiling and running the tests we will get quite a view `runtimer error`s looking like this:

{% highlight bash %}
/usr/include/c++/9/bits/vector.tcc:232:24: runtime error: member access within misaligned address 0x7ffeb5d7cd11 for type 'struct vector', which requires 8 byte alignment
0x7ffeb5d7cd11: note: pointer points here
 00 00 00  01 10 0b 57 e1 b0 55 00  00 18 0b 57 e1 b0 55 00  00 18 0b 57 e1 b0 55 00  00 00 00 00 00
{% endhighlight bash %}

## A somewhat practical example

In order to show you, that alignment issues may have serious consequences even on x86, I'd like to show you this somewhat
contrived but instructive example.

{% highlight cpp %}
#include <new>

struct Vector4i {
    int x;
    int y;
    int z;
    int w;
    long double xx;
};

class bad_optional {
    bool mHasValue;
    char mBuffer[sizeof(Vector4i)];

   public:
    bad_optional(const Vector4i& v) : mHasValue(true) {
        new (&mBuffer) Vector4i(v);
    }
    const Vector4i& value() const {
        return *reinterpret_cast<const Vector4i*>(&mBuffer);
    }
};

int main() {
    const Vector4i v = {1, 2, 3, 4};
    const bad_optional bv(v);
}
{% endhighlight cpp %}

If we compile this program with the flags `-O1 -msse4.2 -fno-inline` using GCC and run it, we will get a
[segmentation fault](https://en.wikipedia.org/wiki/Segmentation_fault). (Link to [Compiler Explorer](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:2,endLineNumber:27,positionColumn:2,positionLineNumber:27,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:'%23include+%3Cnew%3E%0A%0Astruct+Vector4i+%7B%0A++++int+x%3B%0A++++int+y%3B%0A++++int+z%3B%0A++++int+w%3B%0A++++long+double+xx%3B%0A%7D%3B%0A%0Aclass+bad_optional+%7B%0A++++bool+mHasValue%3B%0A++++char+mBuffer%5Bsizeof(Vector4i)%5D%3B%0A%0A+++public:%0A++++bad_optional(const+Vector4i%26+v)+:+mHasValue(true)+%7B%0A++++++++new+(%26mBuffer)+Vector4i(v)%3B%0A++++%7D%0A++++const+Vector4i%26+value()+const+%7B%0A++++++++return+*reinterpret_cast%3Cconst+Vector4i*%3E(%26mBuffer)%3B%0A++++%7D%0A%7D%3B%0A%0Aint+main()+%7B%0A++++const+Vector4i+v+%3D+%7B1,+2,+3,+4%7D%3B%0A++++const+bad_optional+bv(v)%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:37.90323141061021,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g122,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:eigen,ver:'340')),options:'-O1+-msse4.2+-fno-inline',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+12.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:28.76343525605646,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+gcc+12.1',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+gcc+12.2+(Compiler+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4); please note, that `Program returned: 139` means, that the [program segfaulted](https://komodor.com/learn/sigsegv-segmentation-faults-signal-11-exit-code-139/))

This happens because GCC uses a specific instruction to copy `Vector4i`:
[MOVDQA](https://www.felixcloutier.com/x86/movdqa:vmovdqa32:vmovdqa64). This instruction can move four integers
(32-bit) at once, if they are 128-bit aligned. `Vector4i` is supposed to be 128-bit (because of its `long double` member) aligned and contains four
integers, so the compiler is actually allowed to do that. Unfortunately `mBuffer` is *not* 128-bit aligned.
The documentation of MOVDQA states clearly, that ins such a case a *general proctection exection* is thrown,
which will result in a [segfault under Linux](https://stackoverflow.com/a/11985287).

Actually this example is probably not that contrived: it turns out, that we can encounter [similar issues](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:13,endLineNumber:19,positionColumn:13,positionLineNumber:19,selectionStartColumn:13,selectionStartLineNumber:19,startColumn:13,startLineNumber:19),source:'%23include+%3Cnew%3E%0A%23include+%3CEigen/Dense%3E%0A%0Ausing+Vector4i+%3D+Eigen::Vector4i%3B%0A%0Aclass+bad_optional+%7B%0A++++bool+mHasValue%3B%0A++++char+mBuffer%5Bsizeof(Vector4i)%5D%3B%0A%0A+++public:%0A++++bad_optional(const+Vector4i%26+v)+:+mHasValue(true)+%7B%0A++++++++new+(%26mBuffer)+Vector4i(v)%3B%0A++++%7D%0A++++const+Vector4i%26+value()+const+%7B%0A++++++++return+*reinterpret_cast%3Cconst+Vector4i*%3E(%26mBuffer)%3B%0A++++%7D%0A%7D%3B%0A%0Aint+main()+%7B%0A++++const+Vector4i+v+%3D+%7B1,+2,+3,+4%7D%3B%0A++++const+bad_optional+bv(v)%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:37.90323141061021,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g122,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:eigen,ver:'340')),options:'-O1+-msse4.2+-fno-inline',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+12.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:28.76343525605646,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+gcc+12.1',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+gcc+12.2+(Compiler+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4)
, if we use `Eigen::Vector4i` which is provided by the popular linear algebra library [Eigen](https://en.wikipedia.org/wiki/Eigen_(C%2B%2B_library)).

## Fixing the alignment with an workaround

So, how can we fix our optional then? First of all, we need to go back to alignment rules above. In general we can
identify two cases here: a type is

* either "composed" like `struct`s, `class`es, `union`s and arrays -- these types inherit their alignment from their
  members --
* or "build-in", which means that it's alignment is defined by it's size.

As all pointer types have the same size, the set of different alignment requirements is finite. Therefore there must be
a maximal alignment requirement, which then would be suitable for each and every standard compliant type.

Beside this chain of arguments, there must be such a maximum alignment requirement, simply because of
[std::malloc](https://en.cppreference.com/w/cpp/memory/c/malloc). `malloc` just returns a `void` pointer --
it does not know the type which the allocated memory is supposed to be used for. As we can pass the return value of
`malloc` to the placement-new operator, the allocated memory must be aligned in a way which fits for each an every type.
The [cppreference documentation of std::malloc](https://en.cppreference.com/w/cpp/memory/c/malloc) says exactly that:

> If allocation succeeds, returns a pointer to the lowest (first) byte in the allocated memory block that is suitably aligned for any scalar type (at least as strictly as std::max_align_t).

So there is not only a maximum alignment requirement, there is actually a type
[std::max_align_t](https://en.cppreference.com/w/cpp/types/max_align_t), which is defined to have the most strict
alignment requirements possible. Unfortunately it has been introduced in C++11, which we can not use here.

But we can implement something similar on our own. We already know, that a `union` always inherits it's alignment
requirements from the member with the most strict alignment requirements. Therefore we can implement `std::max_align_t`
using a `union` which has a member for each "build-in" type.

{% highlight cpp %}
union max_align_t {
  long long ll;
  long double ld;
};
{% endhighlight cpp %}

This list of types is not comprehensive, but it should fit our needs:

* `long long` is the largest integer type on all platforms (64 bit, `long` might have the same size, but no other integer type
  is larger).
* `long long` will be at least as large as the processors [register width](https://en.wikipedia.org/wiki/Processor_register),
   so there should not be any need for adding any pointer types here.
* `long double` is the largest floating point (either 128 or 64 bit).

If we have a sneak peek on
[GCCs implementation](https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/gcc/ginclude/stddef.h#L415)
we see, that we are actually not far off of it. I suppse that we got as far as one may get in Standard C++ without using
[compiler intrinsics](https://learn.microsoft.com/en-us/cpp/intrinsics/compiler-intrinsics?view=msvc-170).

We can now use this `max_align_t` to fix the alignment of `mBuffer`: we put them both together in a `union`.

{% highlight cpp %}
template <typename T>
class optional {
 private:
  //...
  union max_align_t {
    long long ll;
    long double ld;
  };

  union {
    char mBuffer[sizeof(T)];
    max_align_t mDummy;
  };
  //...
};
{% endhighlight %}

The `union` will inherit the alignment requirements of `max_align_t` and therefore `mBuffer` will always fulfill the
alignment requirements of `max_align_t`, too. As this is the maximal possible alignment for all types (in C++98), it
will suffice for each and every `T` we might come up with. Our tests will succeed without any issues -- even if we
enable the undefined behavior sanitizer. Even our example above will work, 
[if we adapt if accordingly](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:24,endLineNumber:15,positionColumn:24,positionLineNumber:15,selectionStartColumn:24,selectionStartLineNumber:15,startColumn:24,startLineNumber:15),source:'%23include+%3Cnew%3E%0A%0Astruct+Vector4i+%7B%0A++++int+x%3B%0A++++int+y%3B%0A++++int+z%3B%0A++++int+w%3B%0A++++long+double+xx%3B%0A%7D%3B%0A%0Aclass+bad_optional+%7B%0A++++bool+mHasValue%3B%0A++++union+max_align_t+%7B%0A++++++++long+long+ll%3B%0A++++++++long+double+ld%3B%0A++++%7D%3B%0A++++union+%7B%0A++++++++max_align_t+mDummy%3B%0A++++++++char+mBuffer%5Bsizeof(Vector4i)%5D%3B%0A++++%7D%3B%0A%0A+++public:%0A++++bad_optional(const+Vector4i%26+v)+:+mHasValue(true)+%7B%0A++++++++new+(%26mBuffer)+Vector4i(v)%3B%0A++++%7D%0A++++const+Vector4i%26+value()+const+%7B%0A++++++++return+*reinterpret_cast%3Cconst+Vector4i*%3E(%26mBuffer)%3B%0A++++%7D%0A%7D%3B%0A%0Aint+main()+%7B%0A++++const+Vector4i+v+%3D+%7B1,+2,+3,+4%7D%3B%0A++++const+bad_optional+bv(v)%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:37.90323141061021,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g122,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:eigen,ver:'340')),options:'-O1+-msse4.2+-fno-inline',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+12.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:28.76343525605646,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+gcc+12.1',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+gcc+12.2+(Compiler+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4).

## Conclusion

Ok, this was probably the most complex post so far: we had to deal with *data alignment* and it's rules. We had to
understand C++'s alignment rules and how they affect our code -- especially with respect to undefined behavior.
But finally we could derive a suitable solution for the problem at hand: we made sure, that the alignment requirements
for all `T`s are met.

But I already called this solution a hack and I did it for a reason. With this solution, any `optional<T>` will always
be `sizeof(max_align_t) * 2` bytes large. On x86, this will usually be 
[32 bytes](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:48,endLineNumber:14,positionColumn:48,positionLineNumber:14,selectionStartColumn:48,selectionStartLineNumber:14,startColumn:48,startLineNumber:14),source:'%23include+%3Ciostream%3E%0A%0Astruct+optional+%7B%0A++++bool+mHasValue%3B%0A++++union+max_align_t+%7B%0A++++++++long+long+ll%3B%0A++++++++long+double+ld%3B%0A++++%7D%3B%0A++++max_align_t+dummy%3B%0A%7D%3B%0A%0Aint+main()+%7B%0A++++std::cout+%3C%3C+sizeof(optional::max_align_t)+%3C%3C+std::endl%3B%0A++++std::cout+%3C%3C+sizeof(optional)+%3C%3C+std::endl%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:37.90323141061021,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g122,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:eigen,ver:'340')),options:'',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+12.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:28.76343525605646,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+gcc+12.1',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+gcc+12.2+(Compiler+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4)!
In many cases, this is quite wasteful. A `struct` containing a `bool` and a `int` is only
[8 bytes large](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:16,endLineNumber:5,positionColumn:16,positionLineNumber:5,selectionStartColumn:16,selectionStartLineNumber:5,startColumn:16,startLineNumber:5),source:'%23include+%3Ciostream%3E%0A%0Astruct+optional+%7B%0A++++bool+mHasValue%3B%0A++++int+mValue%3B%0A%7D%3B%0A%0Aint+main()+%7B%0A++++std::cout+%3C%3C+sizeof(optional)+%3C%3C+std::endl%3B%0A%7D'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:37.90323141061021,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g122,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:eigen,ver:'340')),options:'',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1,tree:'1'),l:'5',n:'0',o:'x86-64+gcc+12.2+(C%2B%2B,+Editor+%231,+Compiler+%231)',t:'0')),k:28.76343525605646,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+gcc+12.1',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+gcc+12.2+(Compiler+%231)',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4).

Therefore we will try to come up with another solution in the upcoming post.
