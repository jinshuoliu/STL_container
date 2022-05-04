/**
 * @brief 
 * 
 * @version G2.9
 * @tparam Key 
 * @tparam T 
 * @tparam Compare 
 * @tparam Alloc 
 */
template <class Key, class T, Compare = less<Key>, class Alloc = alloc>
class map {
public:
  typedef Key key_type;
  typedef T data_type;
  typedef T mapped_type;
  // 这里key是const的
  typedef pair<const Key, T> value_type;
  typedef Compare key_compare;
private:
  // 也是底层红黑树,和set几乎没啥区别
  typedef rb_tree<key_type, value_type, selectlst<value_type>, key_compare, Alloc> rep_type;
  rep_type t;
public:
  // 这里返回的是iterator
  typedef typename rep_type::iterator iterator;
};

template <class Arg, class Result>
struct unary_function {
  typedef Arg argument_type;
  typedef Result result_type;
};
template <class T>
struct selectlst : public unary_function<Pair, typename Pair::first_type> {
  const typename Pair::first_type& operator()(const Pair& X) const {
    return x.first;
  }
};

/**
 * @brief 
 * 
 * @version VC6
 * @tparam _K 
 * @tparam _Ty 
 * @tparam _Pr 
 * @tparam _A 
 */
template<class _K, class _Ty, class _Pr=less<_K>, class _A=allocator<_Ty>>
class map {
public:
  typedef map<_K, _Ty, _Pr, _A> _Myt;
  typedef pair<const _K, _Ty> value_type;

  struct _Kfn : public unary_function<value_type, _K> {
    const _K& operator()(const value_type& _X) const {
      return (_X.first);
    }
  }

  rtpedef _Tree<_K, value_type, _Kfn, _Pr, _A> _Imp;

protected:
  _Imp _Tr;
};