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
