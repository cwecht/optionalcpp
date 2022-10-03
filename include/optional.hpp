#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

class bad_optional_access : public std::exception {};

template <typename T>
class optional {
 public:
  optional()
      : mHasValue(false) {}

  optional(const T& value)
      : mHasValue(true) {
    constructValue(value);
  }

  optional(const optional& other)
      : mHasValue(other.mHasValue) {
    if (other.mHasValue) {
      constructValue(*other);
    }
  }

  optional& operator=(const optional& other) {
    if (mHasValue && other.mHasValue) {
      *(*this) = *other;
    } else if (other.mHasValue) {
      constructValue(*other);
    } else if (mHasValue) {
      destructValue();
    }
    mHasValue = other.mHasValue;
    return *this;
  }

  ~optional() {
    if (mHasValue) {
      destructValue();
    }
  }

  bool has_value() const {
    return mHasValue;
  }

  operator bool() const {
    return mHasValue;
  }

  const T& value() const {
    throwInCaseOfBadAccess();
    return *(*this);
  }

  T& value() {
    throwInCaseOfBadAccess();
    return *(*this);
  }

  const T& operator*() const {
    return *reinterpret_cast<const T*>(&mBuffer);
  }

  T& operator*() {
    return *reinterpret_cast<T*>(&mBuffer);
  }

  const T* operator->() const {
    return &(*(*this));
  }

  T* operator->() {
    return &(*(*this));
  }

  friend bool operator==(const optional& a, const optional& b) {
    if (a.mHasValue && b.mHasValue) {
      return *a == *b;
    }
    return !a.mHasValue && !b.mHasValue;
  }

  template <typename U>
  friend bool operator==(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return false;
    }
    return *a == b;
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
      return *a < *b;
    }
    return !a.mHasValue && b.mHasValue;
  }

  template <typename U>
  friend bool operator<(const optional& a, const U& b) {
    if (not a.mHasValue) {
      return true;
    }
    return *a < b;
  }

  template <typename U>
  friend bool operator<(const U& a, const optional& b) {
    if (not b.mHasValue) {
      return false;
    }
    return a < *b;
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

  union max_align_t {
    long long ll;
    long double ld;
  };

  union {
    char mBuffer[sizeof(T)];
    max_align_t mDummy;
  };

  void throwInCaseOfBadAccess() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
  }

  void constructValue(const T& other) {
    new (&mBuffer) T(other);
  }

  void destructValue() {
    reinterpret_cast<T*>(&mBuffer)->~T();
  }
};

#endif  // OPTIONALCPP_OPTIONAL_HPP
