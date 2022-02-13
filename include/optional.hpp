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

  friend bool operator ==(optional_unsigned_int a, optional_unsigned_int b) {
    if (a.mHasValue && b.mHasValue) {
        return a.mValue == b.mValue;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  friend bool operator !=(optional_unsigned_int a, optional_unsigned_int b) {
    return !(a == b);
  }

private:
  bool mHasValue;
  unsigned int mValue;
};

#endif // OPTIONALCPP_OPTIONAL_HPP
