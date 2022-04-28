// array使用的时候需要指定类型和大小
template <typename _Tp, std::size_t _Nm>
struct array {
  // 这里是数据的类型
  typedef _Tp value_type;
  typedef _Tp* pointer;
  typedef value_type* iterator;

  // 至少存一个呗
  value_type _M_instance[_Nm ? _Nm : 1];

  iterator begin() {
    return iterator(&_M_instance[0]);
  }

  iterator end() {
    return iterator(&_M_instance[_Nm]);
  }
  ...
};


/**
 * @brief 
 * 
 * @version G4.9
 * @tparam _Tp 
 * @tparam _Nm 
 */
template <typename _Tp, std::size_t _Nm>
struct array {
  // 这个重命名以下传入的类型名的习惯，可以使用一下
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef value_type* iterator; // 这里和pointer一样，array本身就是个很简单的数组而已
  typedef std::size_t size_type;
  
  typedef _GLIBCXX_STD_C::__array_traits<_Tp, _Nm> _AT_Type;
  typename _AT_Type::_Type _M_elems;

  // noecvept:该关键字告诉编译器，函数中不会发生异常,这有利于编译器对程序做更多的优化。
  iterator begin() noexcept {
    return iterator(data());
  }

  iterator end() noexcept {
    return iterator(data() + _Nm);
  }

  constecpr size_type size() const noexcept {
    return _Nm;
  }

  refernece operator[](size_type __n) noexcept {
    return _AT_Type::_S_ref(_M_elems, __n);
  }

  reference at(size_type __n) {
    if(__n >= _Nm) std::__throw_out_of_range_fmt(...);
    return _AT_Type::_S_ref(_M_elems, __n);
  }

  pointer data() noexcept {
    return std::__addressof(_AT_Type::_S_ref(_M_elems, 0));
  }
  ...
};

template <typename _Tp, std::size_t _Nm>
struct __array_traits {
  typedef _Tp _Type[_Nm];

  static constexpr _Tp& _S_ref(const _Type& __t, std::size_t __n) noexcept {
    return const_cast<_Tp&>(__t[__n]);
  }
};