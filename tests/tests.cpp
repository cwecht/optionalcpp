#include "catch_with_main.hpp"

#include "optional.hpp"

TEST_CASE("A default constructed optional has no value.") {
  optional_unsigned_int x;
  REQUIRE(x.has_value() == false);
}

TEST_CASE(
    "An optional constructed with a value has a value and stores the value.") {
  unsigned int anyValue = 10;
  optional_unsigned_int x(anyValue);
  REQUIRE(x.has_value() == true);
  REQUIRE(x.value() == anyValue);
}

TEST_CASE("Two optionals without a value are equal.") {
  optional_unsigned_int x;
  optional_unsigned_int y;
  REQUIRE(x == y);
  REQUIRE(y == x);
  REQUIRE(!(x != y));
  REQUIRE(!(y != x));
}

TEST_CASE("Two optionals with equal values are equal.") {
  unsigned int anyValue = 5;
  optional_unsigned_int x(anyValue);
  optional_unsigned_int y(anyValue);
  REQUIRE(x == y);
  REQUIRE(y == x);
  REQUIRE(!(x != y));
  REQUIRE(!(y != x));
}

TEST_CASE("An optional without a value and an optional with a value are unequal.") {
  optional_unsigned_int x;
  unsigned int anyValue = 1;
  optional_unsigned_int y(anyValue);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
  REQUIRE(x != y);
  REQUIRE(y != x);
}

TEST_CASE("Two optionals with unequal values are unequal.") {
  unsigned int anyValue = 5;
  unsigned int anyOtherValue = 6;
  optional_unsigned_int x(anyValue);
  optional_unsigned_int y(anyOtherValue);
  REQUIRE(!(x == y));
  REQUIRE(!(y == x));
  REQUIRE(x != y);
  REQUIRE(y != x);
}

TEST_CASE("An optional with a value and an unsigned value with the same value are equal") {
  unsigned int anyValue = 1;
  optional_unsigned_int x(anyValue);
  REQUIRE(x == anyValue);
  REQUIRE(anyValue == x);
  REQUIRE(!(x != anyValue));
  REQUIRE(!(anyValue != x));
}
