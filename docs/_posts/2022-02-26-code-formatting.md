---
layout: post
title:  "Interlude: code formatting"
date:   2022-02-26 20:31:00 +0100
---

## Code Formatting

As our optional implementation grows and thrives, a consistent code format across the code base becomes desirable more
and more. We could try to do archive it manually but this becomes quickly arduous and is a pretty gratuitous endeavor 
in the presence of tools like [clang-format](https://clang.llvm.org/docs/ClangFormat.html).

## The Style File

`clang-format` can be configured using a so-called *style file*. This is filed called `.clang-format`, which contains
the `clang-format` configuration. By default, `clang-format` will search for this file starting from the folder, which
contains the current file to format. If such a file is not found there, `clang-format` will recursively search for style
files in the parent folders. So we can define our code formatting style by putting such a style file in the root folder
or our git repository (other open source projects like
[cmake](https://github.com/Kitware/CMake/blob/master/.clang-format) do it the same way).

{% highlight conf %}
---
BasedOnStyle: Google
AllowAllConstructorInitializersOnNextLine: 'false'
AllowShortFunctionsOnASingleLine: Empty
BreakConstructorInitializers: BeforeComma
ConstructorInitializerAllOnOneLineOrOnePerLine: 'false'
IndentWidth: '2'
UseTab: Never
...
{% endhighlight %}

The style is based on the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html#Formatting). It is
a good starting point in my opinion: I had not too change too many options.

- `AllowAllConstructorInitializersOnNextLine` and `ConstructorInitializerAllOnOneLineOrOnePerLine` both set to `false`
  will reformat each initializer in a constructor on its own line.
- `BreakConstructorInitializers` set to `BeforeComma` will place the colon and the commas of a constructors initializer
  list in the same column, just like this:
{% highlight cpp %}
optional_unsigned_int(unsigned int value)
    : mHasValue(true)
    , mValue(value) {}
{% endhighlight %}
- `AllowShortFunctionsOnASingleLine` set to `Empty` will only allow empty functions on a single line. Any other
  function will be placed on multiple lines.
- `UseTaks` set to `Never` will prohibit the use of tabs. Spaces have always the same size on the screen (one
  character), tabs don't. Especially if tabs and spaces are used at the same time for formatting, the visible format
  will depend on the editors configuration. There for spaces are more "portable" than tabs regarding formatting of code.
- `IndentWith` is set to `2`. A very common value for it is `4` but I personally see no reason to waste so much
  horizontal space for indentation.

All these adaptions are very subjective but I think, that these are at least somewhat reasonable. If you want to play
around with different formatting options, I can recommend
[this web based tool](https://zed0.co.uk/clang-format-configurator/), which makes finding the "right" configuration much
easier.
The most important thing though is to use a code formatter at all and stop caring about formatting.

## Formatting the Code

Before formatting our code we need to make sure to agree on the same version `clang-format` as the developers don't
guarantee at all that the same style files will result in the same formatted code for different version of
`clang-format`. In my experience you should assume that a new version of `clang-format`  will reformat your code.
I decided to use `clang-format-10` for this project for no particular reason. This version is somewhat "recent", but old
enough to be accessible on older platforms without any hassle.

For formatting a file in our repository we can now use this command:

{% highlight bash %}
clang-format-10 -i <file>
{% endhighlight %}

But usually one will not use clang-format manually. The very most IDEs or editors have support for clang-tidy. For
example, I am using the vim plugin [vim-clang-format](ttps://github.com/rhysd/vim-clang-format.git).

## Conclusion

Strictly speaking, this post had not too much to do with implementing an optional in C++. But code formatting is still
an important matter. Choosing a formatting style is an fundamental decision of a project (even if you choose not to
choose). I wanted to get the issue formatting out of our way so that we may stop worrying about it an can focus on the
actual implementation, which we will continue in the upcoming posts.
