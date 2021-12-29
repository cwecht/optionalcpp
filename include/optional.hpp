#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

struct optional_unsigned_int {

  optional_unsigned_int()
    : is_set(false) {}

  optional_unsigned_int(unsigned int v)
    : is_set(true)
    , value(v) {};

  bool is_set;
  unsigned int value;
};

#endif // OPTIONALCPP_OPTIONAL_HPP
