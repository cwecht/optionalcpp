---
layout: post
title:  "What is optional?"
date:   2021-12-05 18:45:00 +0100
---

Let's say you want to write an command line application, which takes a bunch or parameters. Some of these parameters are
(unsigned) integers, which may or may not be passed by the user to the application. Maybe you develop a little game and
you want to be able to pass the windows width and height as separate parameters `--width` and `--height`. These two
values could be bundles up (beside maybe multiple others) in a struct `Configuration` like this:

{% highlight cpp %}
struct Configuration {
  unsigned int width;
  unsigned int height;
};
{% endhighlight %}

Now, what you want to give the user the *option* - but not the *obligation* - to pass these values to the application.
How would you do that? You could define some default values and use them instead. But maybe you don't want to have
default values, because you maybe want the application to start in full screen mode. With this intention in mind you might
introduce boolean flags `width_is_set` and `height_is_set`, which indicate whether the corresponding value has been set
by the user.

{% highlight cpp %}
struct Configuration {
  bool width_is_set;
  unsigned int width;

  bool height_is_set;
  unsigned int height;
};
{% endhighlight %}

But now we can see a pattern here: we have an *unsigned integer* bundled up with a *booelan* value. It might make sense
to put them into a struct `optional_unsigned_int`.

{% highlight cpp %}
struct optional_unsigned_int {
  bool is_set;
  unsigned int value;
};

struct Configuration {
  optional_unsigned_int width;
  optional_unsigned_int height;
};
{% endhighlight %}

An there we have it: a concrete instance of an optional. `std::optional` (and any other optional, which is part of any
library I am aware of) has a different interface, is -- of cause -- more complex and has more features, but the basic
idea is the same: bundle up a value and a boolean, which indicates, whether the value is *valid* or not. *Valid* means
in this case, that `value` has been set and that is makes sense to read it. If the value is *invalid*, it has not been 
set and it makes no sense to read it.

This is -- abviously -- only *one* example in which the use of an optional may be sensivle; there are many other reasonable
 use cases -- especially if we want to embrace a more *functional* style of programming (*functional* as in *functional
 programming*). I have  chosen this CLI arguments example because I think, that it should be appealing for every kind of
programming or programming style. 

### How to proceed?

Obviously, this post is only the very first step towards a fully featured optional implementation. The next steps must
be

1. make optional a proper class,
2. generalize it (which means: transform it to a class template, which may hold an arbitrary type) and
3. evolve the implementation from C++03 to C++11 and so on.

These steps will be approached in future blog posts beside setting up cmake and catch2.
