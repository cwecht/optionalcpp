#include "catch_with_main.hpp"
#include "optional.hpp"

typedef optional<unsigned int> optional_unsigned_int;

TEST_CASE("A default constructed optional has no value.") {
  const optional_unsigned_int x;
  REQUIRE(x.has_value() == false);
}

TEST_CASE("An default constructed optional converts to 'false'.") {
  const optional_unsigned_int x;
  REQUIRE(!x);
}

TEST_CASE("An optional constructed with a value converts to 'true'.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(x);
}

TEST_CASE(
    "An optional constructed with a value has a value and stores the value.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(x.has_value() == true);
  REQUIRE(x.value() == anyValue);
}

TEST_CASE("An optional constructed with a value can be dereferenced.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(*x == anyValue);
}

struct A {
  unsigned int x;
};

TEST_CASE("An optional constructed with a value can access members directly.") {
  unsigned int anyValue = 10;
  A anyStructValue = {anyValue};
  const optional<A> a(anyStructValue);
  REQUIRE(a->x == anyValue);
}

TEST_CASE("An non-const optional's value can be mutated.") {
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

TEST_CASE("Two optionals without a value are equal.") {
  optional_unsigned_int x;
  optional_unsigned_int y;
  REQUIRE(x == y);
  REQUIRE(y == x);
  REQUIRE(x >= y);
  REQUIRE(y >= x);
  REQUIRE(x <= y);
  REQUIRE(y <= x);
  REQUIRE(!(x != y));
  REQUIRE(!(y != x));
  REQUIRE(!(x < y));
  REQUIRE(!(y < x));
  REQUIRE(!(x > y));
  REQUIRE(!(y > x));
}

TEST_CASE("Two optionals with equal values are equal.") {
  unsigned int anyValue = 5;
  optional_unsigned_int x(anyValue);
  optional_unsigned_int y(anyValue);
  REQUIRE(x == y);
  REQUIRE(y == x);
  REQUIRE(x <= y);
  REQUIRE(y <= x);
  REQUIRE(x >= y);
  REQUIRE(y >= x);
  REQUIRE(!(x != y));
  REQUIRE(!(y != x));
  REQUIRE(!(x < y));
  REQUIRE(!(y < x));
  REQUIRE(!(x > y));
  REQUIRE(!(y > x));
}

TEST_CASE(
    "An optional with a value and an unsigned value with the same value are "
    "equal.") {
  unsigned int anyValue = 1;
  optional_unsigned_int x(anyValue);
  REQUIRE(x == anyValue);
  REQUIRE(anyValue == x);
  REQUIRE(x >= anyValue);
  REQUIRE(anyValue >= x);
  REQUIRE(x <= anyValue);
  REQUIRE(anyValue <= x);
  REQUIRE(!(x != anyValue));
  REQUIRE(!(anyValue != x));
  REQUIRE(!(x < anyValue));
  REQUIRE(!(anyValue < x));
  REQUIRE(!(x > anyValue));
  REQUIRE(!(anyValue > x));
}

TEST_CASE(
    "An optional without a value is less then an optional with a value (they "
    "are unequal).") {
  optional_unsigned_int x;
  unsigned int anyValue = 10;
  optional_unsigned_int y(anyValue);
  REQUIRE(x < y);
  REQUIRE(y > x);
  REQUIRE(x <= y);
  REQUIRE(y >= x);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
  REQUIRE(x != y);
  REQUIRE(y != x);
}

TEST_CASE(
    "An optional 'x' is less than an optional 'y' if the value of 'x' is "
    "smaller than the value of 'y' (they are unequal).") {
  unsigned int anyValueX = 5;
  unsigned int anyValueY = 6;
  REQUIRE(anyValueX < anyValueY);
  optional_unsigned_int x(anyValueX);
  optional_unsigned_int y(anyValueY);
  REQUIRE(x < y);
  REQUIRE(y > x);
  REQUIRE(x <= y);
  REQUIRE(y >= x);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
  REQUIRE(x != y);
  REQUIRE(y != x);
}

TEST_CASE("An optional and its copy are equal.") {
  SECTION("copy constructor without a value") {
    optional_unsigned_int x;
    optional_unsigned_int y(x);
    REQUIRE(x == y);
  }

  SECTION("copy constructor with a value") {
    unsigned int anyValueX = 5;
    optional_unsigned_int x(anyValueX);
    optional_unsigned_int y(x);
    REQUIRE(x == y);
  }

  SECTION("copy constructor without a value (non trivial copy constructor)") {
    optional<std::string> x;
    optional<std::string> y(x);
    REQUIRE(x == y);
  }

  SECTION("copy constructor with a value (non trivial copy constructor)") {
    std::string anyValueX = "value";
    optional<std::string> x(anyValueX);
    optional<std::string> y(x);
    REQUIRE(x == y);
  }

  SECTION("copy assignment without a value") {
    optional_unsigned_int x;
    optional_unsigned_int y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assignment with a value") {
    unsigned int anyValueX = 5;
    optional_unsigned_int x(anyValueX);
    optional_unsigned_int y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assignment to two optionals") {
    int anyValueX = 5;
    optional<int> x(anyValueX);
    optional<int> y;
    optional<int> z;
    z = y = x;
    REQUIRE(x == y);
    REQUIRE(x == z);
  }

  SECTION(
      "copy assign optional without a value to an optional without a value") {
    optional<std::vector<int> > x;
    optional<std::vector<int> > y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional with a value to an optional without a value") {
    std::vector<int> anyValueX;
    anyValueX.push_back(1);
    anyValueX.push_back(2);
    anyValueX.push_back(3);
    optional<std::vector<int> > x(anyValueX);
    optional<std::vector<int> > y;
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional without a value to an optional with a value") {
    optional<std::vector<int> > x;
    std::vector<int> anyValueY;
    anyValueY.push_back(1);
    anyValueY.push_back(2);
    anyValueY.push_back(3);
    optional<std::vector<int> > y(anyValueY);
    y = x;
    REQUIRE(x == y);
  }

  SECTION("copy assign optional with a value to an optional with a value") {
    std::vector<int> anyValueX;
    anyValueX.push_back(1);
    anyValueX.push_back(2);
    anyValueX.push_back(3);
    optional<std::vector<int> > x(anyValueX);

    std::vector<int> anyValueY;
    anyValueY.push_back(1);
    anyValueY.push_back(3);
    optional<std::vector<int> > y(anyValueY);
    y = x;
    REQUIRE(x == y);
  }
}

struct CopyCounting {
  int const copyCount;

  CopyCounting()
      : copyCount(0) {}

  CopyCounting(const CopyCounting& other)
      : copyCount(other.copyCount + 1) {}
};

TEST_CASE(
    "During initialization of an optional with a value and reading from it, "
    "the value type is copied only once.") {
  CopyCounting c;
  REQUIRE(c.copyCount == 0);
  const optional<CopyCounting> x(c);
  REQUIRE(x.value().copyCount == 1);
}

struct NonDefaultConstructable {
  NonDefaultConstructable(int x){};
};

TEST_CASE(
    "An optional of a non default constructable type can be default "
    "constructed.") {
  const optional<NonDefaultConstructable> x;
  REQUIRE(x.has_value() == false);
}

struct CheckedDestructorCalls {
  CheckedDestructorCalls() {
    ++missingDestructorCalls;
  }
  CheckedDestructorCalls(const CheckedDestructorCalls&) {
    ++missingDestructorCalls;
  }
  ~CheckedDestructorCalls() {
    --missingDestructorCalls;
  }

  static int missingDestructorCalls;
};

int CheckedDestructorCalls::missingDestructorCalls = 0;

TEST_CASE("An optional with a value destructs the value during destruction.") {
  {
    const optional<CheckedDestructorCalls> x =
        optional<CheckedDestructorCalls>(CheckedDestructorCalls());
    REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 1);
  }
  REQUIRE(CheckedDestructorCalls::missingDestructorCalls == 0);
}

TEST_CASE("value() will throw if it is called on an optional without a value") {
  const optional<int> empty;
  REQUIRE_THROWS_AS(empty.value(), bad_optional_access);
}

struct GlobalCopyCounting {
  GlobalCopyCounting() {}

  GlobalCopyCounting(const GlobalCopyCounting&) {
    ++copyCount;
  }

  static int copyCount;
};

int GlobalCopyCounting::copyCount = 0;

bool operator==(const GlobalCopyCounting&, const GlobalCopyCounting&) {
  return true;
}

bool operator<(const GlobalCopyCounting&, const GlobalCopyCounting&) {
  return true;
}

TEST_CASE(
    "While comparing an optional with a value the value is not copied (because "
    "no temporary is created)") {
  const GlobalCopyCounting ref;
  const optional<GlobalCopyCounting> opt(ref);
  GlobalCopyCounting::copyCount = 0;

  optional<std::string> op("hello");

  bool val = op == "he";

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

TEST_CASE(
    "If two optionals with values are swapped, their values are swapped.") {
  unsigned int anyValueX = 10;
  unsigned int anyValueY = 2;
  optional_unsigned_int x(anyValueX);
  optional_unsigned_int y(anyValueY);

  x.swap(y);

  REQUIRE(*x == anyValueY);
  REQUIRE(*y == anyValueX);
}

TEST_CASE("If two optionals with no values are swapped, they stay empty.") {
  optional_unsigned_int x;
  optional_unsigned_int y;

  x.swap(y);

  REQUIRE(not x.has_value());
  REQUIRE(not y.has_value());
}

TEST_CASE(
    "If an optional with a value and an optional without a value are swapped, "
    "the value is transferred.") {
  unsigned int anyValueX = 10;
  optional_unsigned_int x(anyValueX);
  optional_unsigned_int y;

  SECTION("this has value") {
    x.swap(y);
  }
  SECTION("this has value") {
    y.swap(x);
  }

  REQUIRE(not x.has_value());
  REQUIRE(y.has_value());
  REQUIRE(*y == anyValueX);
}

TEST_CASE("Free Function swap called on options swaps the optionals") {
  unsigned int anyValueX = 10;
  optional_unsigned_int x(anyValueX);
  optional_unsigned_int y;

  swap(x, y);

  REQUIRE(not x.has_value());
  REQUIRE(y.has_value());
  REQUIRE(*y == anyValueX);
}

TEST_CASE("reset destroys the value of an optional with a value.") {
  int anyValueX = 5;
  optional<int> mValue(anyValueX);
  REQUIRE(mValue);
  mValue.reset();
  REQUIRE(not mValue);
}

TEST_CASE(
    "After calling reset an optional without a value has still a value.") {
  optional<int> mValue;
  REQUIRE(not mValue);
  mValue.reset();
  REQUIRE(not mValue);
}
