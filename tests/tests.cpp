#include "catch_with_main.hpp"

#include "optional.hpp"

TEST_CASE("A default constructed optional is not set.") {
  optional_unsigned_int x;
  REQUIRE(x.is_set == false);
}

TEST_CASE("An optional constructed with a value is set and stores the value.") {
  unsigned int anyValue = 10;
  optional_unsigned_int x(anyValue);
  REQUIRE(x.is_set == true);
  REQUIRE(x.value == anyValue);
}
