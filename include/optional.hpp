#ifndef OPTIONALCPP_OPTIONAL_HPP
#define OPTIONALCPP_OPTIONAL_HPP

#include <cstddef>

union max_align_t {
  long long ll;
  long double ld;
};

template <typename T>
struct alignment_probe {
  bool x;
  T probee;
};

template <typename T>
struct alignment_of {
  static const std::size_t value = sizeof(alignment_probe<T>) - sizeof(T);
};

template <std::size_t N>
struct type_with_alignment {
  typedef max_align_t type;
};

template <>
struct type_with_alignment<alignment_of<char>::value> {
  typedef char type;
};

template <>
struct type_with_alignment<alignment_of<short>::value> {
  typedef short type;
};

template <>
struct type_with_alignment<alignment_of<int>::value> {
  typedef int type;
};

template <>
struct type_with_alignment<alignment_of<long>::value> {
  typedef long type;
};

template <std::size_t Size,
          std::size_t Alignment = alignment_of<max_align_t>::value>
struct aligned_storage {
  union {
    char mStorage[Size];
    typename type_with_alignment<Alignment>::type mAlignmentDummy;
  };
};

class bad_optional_access : public std::exception {};

template <typename T>
class optional {
 public:
  optional()
      : mHasValue(false) {}

  optional(const T& value) {
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

  void swap(optional& other) {
    if (this->has_value() and other.has_value()) {
      using std::swap;
      swap(*(*this), *other);
    } else if (this->has_value()) {
      other.constructValue(*(*this));
      this->destructValue();
    } else if (other.has_value()) {
      this->constructValue(*other);
      other.destructValue();
    }
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

  friend void swap(optional& a, optional& b ) {
    a.swap(b);
  }

 private:
  bool mHasValue;

  aligned_storage<sizeof(T), alignment_of<T>::value> mBuffer;

  void throwInCaseOfBadAccess() const {
    if (not mHasValue) {
      throw bad_optional_access();
    }
  }

  void constructValue(const T& other) {
    new (&mBuffer.mStorage) T(other);
    mHasValue = true;
  }

  void destructValue() {
    reinterpret_cast<T*>(&mBuffer.mStorage)->~T();
    mHasValue = false;
  }
};

#endif  // OPTIONALCPP_OPTIONAL_HPP
