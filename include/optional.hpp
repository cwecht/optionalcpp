#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

class bad_optional_access : public std::exception {};

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
    throwInCaseOfBadAccess();
    return mValue;
  }

  T& value() {
    throwInCaseOfBadAccess();
    return mValue;
  }

  const T& operator*() const {
    return mValue;
  }

  T& operator*() {
    return mValue;
  }

  const T* operator->() const {
    return &mValue;
  }

  T* operator->() {
    return &mValue;
  }

  friend bool operator==(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue == b.mValue;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  template <typename U>
  friend bool operator==(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return false;
    }
    return a.mValue == b;
  }

  template <typename U>
  friend bool operator==(const U& a, const optional& b) {
    return b == a;
  }

  friend bool operator!=(const optional& a, const optional& b) {
    return !(a == b);
  }

  template <typename U>
  friend bool operator!=(const optional& a, const U& b) {
    return !(a == b);
  }

  template <typename U>
  friend bool operator!=(const U& a, const optional& b) {
    return !(a == b);
  }

  friend bool operator<(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return a.mValue < b.mValue;
    }
    return !a.mHasValue && b.mHasValue;
  }

  template <typename U>
  friend bool operator<(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return true;
    }
    return a.mValue < b;
  }

  template <typename U>
  friend bool operator<(const U& a, const optional& b) {
    if (not b.mHasValue) {
      return false;
    }
    return a < b.mValue;
  }

  friend bool operator>(const optional& a, const optional& b) {
    return b < a;
  }

  template <typename U>
  friend bool operator>(const optional& a, const U& b) {
    return b < a;
  }

  template <typename U>
  friend bool operator>(const U& a, const optional& b) {
    return b < a;
  }

  friend bool operator>=(const optional& a, const optional& b) {
    return !(a < b);
  }

  template <typename U>
  friend bool operator>=(const optional& a, const U& b) {
    return !(a < b);
  }

  template <typename U>
  friend bool operator>=(const U& a, const optional& b) {
    return !(a < b);
  }

  friend bool operator<=(const optional& a, const optional& b) {
    return !(b < a);
  }

  template <typename U>
  friend bool operator<=(const optional& a, const U& b) {
    return !(b < a);
  }

  template <typename U>
  friend bool operator<=(const U& a, const optional<T>& b) {
    return !(b < a);
  }

 private:
  bool mHasValue;

  struct NoValue {};
  union {
    NoValue mNoValue;
    T mValue;
  };

  void throwInCaseOfBadAccess() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
  }
};

#endif  // OPTIONALCPP_OPTIONAL_HPP
