/**
 * @brief 
 * 
 * @version G2.9
 * @tparam Key 
 * @tparam Compare 
 * @tparam Alloc 
 */
template <class Key, class Compare = less<Key>, class Alloc = alloc>
class set {
public:
  // Key是必须要指定的值，而Compare和Alloc是默认值(当然也可以自己指定)
  typedef Key key_type;
  typedef Key value_type;
  typedef Compare key_compare;
  typedef Compare value_compare;
private:
  // 里面含有一个红黑树
  typedef rb_tree<key_type, value_type, identity<value_type>, key_compare, Alloc> rep_type;
  rep_type t;
public:
  // 这里拿的是const_iterator,也就导致了不可以改变元素的值
  typedef typename rep_type::const_iterator iterator;

  // set的所有底层操作都是用t来完成的，可以把set当作红黑树的适配器
  // 就像是stack、queue与deque的关系
};

template <class Arg, class Result>
struct unary_function {
  typedef Arg argument_type;
  typedef Result result_type;
};
template <class T>
struct identity : public unary_function<T, T> {
  const T& operator()(const T& X) const {
    return x;
  }
};

/**
 * @brief 
 * 
 * @version VC6
 * @tparam _K 
 * @tparam _Pr 
 * @tparam _A 
 */
template <class _K, class _Pr=less<_K>, class _A=allocator<_K>>
class set {
public:
  typedef set<_K, _Pr, _A> _Myt;
  typedef _K value_type;
  // 它相当于G2.9的identity(),可以看到它和identity操作完全一样
  struct _Kfn : public unary_function<value_type, _K> {
    const _K& operator()(const value_type& _X) const {
      return (_X);
    }
  };

  typedef _Pr value_compare;
  typedef _K key_type;
  typedef _Pr key_compare;
  typedef _A allocator_type;
  // 这就相当于那个红黑树了
  typedef _Tree<_K, value_type, _Kfn, _Pr, _A> _Imp;
  ...
protected:
  _Imp _Tr;
};