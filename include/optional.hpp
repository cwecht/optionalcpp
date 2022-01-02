#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

class optional_unsigned_int { 
public:
  optional_unsigned_int() 
    : mHasValue(false) {}

  optional_unsigned_int(unsigned int value)
    : mHasValue(true)
    , mValue(value) {}

  bool has_value() {
    return mHasValue;
  }

  unsigned int value() {
    return mValue;
  }

private:
  bool mHasValue;
  unsigned int mValue;
};

#endif // OPTIONALCPP_OPTIONAL_HPP
