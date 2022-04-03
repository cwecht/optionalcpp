#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

template <typename T>
class optional {
 public:
  optional()
      : mHasValue(false) {}

  optional(T value)
      : mHasValue(true)
      , mValue(value) {}

  bool has_value() const {
    return mHasValue;
  }

  T value() const {
    return mValue;
  }

  friend bool operator==(optional a, optional b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue == b.mValue;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  friend bool operator!=(optional a, optional b) {
    return !(a == b);
  }

  friend bool operator<(optional a, optional b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue < b.mValue;
    }
    return !a.mHasValue && b.mHasValue;
  }

  friend bool operator>(optional a, optional b) {
    return b < a;
  }

  friend bool operator>=(optional a, optional b) {
    return !(a < b);
  }

  friend bool operator<=(optional a, optional b) {
    return !(a > b);
  }

 private:
  bool mHasValue;
  T mValue;
};

#endif  // OPTIONALCPP_OPTIONAL_HPP
