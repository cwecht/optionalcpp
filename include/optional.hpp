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

  optional(const optional& other)
      : mHasValue(other.mHasValue) {
    if (other.mHasValue) {
      new (&mValue) T(other.mValue);
    }
  }

  optional& operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      mValue = other.mValue;
    } else if (other.mHasValue) {
      new (&mValue) T(other.mValue);
    } else if (mHasValue) {
      mValue.~T();
    }
    mHasValue = other.mHasValue;
    return *this;
  }

  ~optional() {
    if (mHasValue) {
      mValue.~T();
    }
  }

  bool has_value() const {
    return mHasValue;
  }

  explicit operator bool() const {
    return mHasValue;
  }

  const T& value() const {
    return mValue;
  }

  const T& operator*() const {
    return mValue;
  }

  const T* operator->() const {
    return &mValue;
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
