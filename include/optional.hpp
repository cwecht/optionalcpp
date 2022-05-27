#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

template <typename T>
class optional {
 public:
  optional()
      : mHasValue(false)
      , mNoValue() {}

  optional(const T& value)
      : mHasValue(true)
      , mValue(value) {}

  bool has_value() const {
    return mHasValue;
  }

  const T& value() const {
    return mValue;
  }

  friend bool operator==(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue == b.mValue;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  friend bool operator!=(const optional& a, const optional& b) {
    return !(a == b);
  }

  friend bool operator<(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue < b.mValue;
    }
    return !a.mHasValue && b.mHasValue;
  }

  friend bool operator>(const optional& a, const optional& b) {
    return b < a;
  }

  friend bool operator>=(const optional& a, const optional& b) {
    return !(a < b);
  }

  friend bool operator<=(const optional& a, const optional& b) {
    return !(a > b);
  }

 private:
  bool mHasValue;

  struct NoValue {};
  union {
    NoValue mNoValue;
    T mValue;
  };
};

#endif  // OPTIONALCPP_OPTIONAL_HPP
