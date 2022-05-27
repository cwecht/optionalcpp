#include "catch_with_main.hpp"
#include "optional.hpp"

typedef optional<unsigned int> optional_unsigned_int;

TEST_CASE("A default constructed optional has no value.") {
  const optional_unsigned_int x;
  REQUIRE(x.has_value() == false);
}

TEST_CASE(
    "An optional constructed with a value has a value and stores the value.") {
  unsigned int anyValue = 10;
  const optional_unsigned_int x(anyValue);
  REQUIRE(x.has_value() == true);
  REQUIRE(x.value() == anyValue);
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
