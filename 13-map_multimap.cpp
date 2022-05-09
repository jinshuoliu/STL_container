template <class T1, class T2>
struct pair {
  typedef T1 first_type;
  typedef T2 second_type;
  T1 first;
  T2 second;

  pair() : first(T1()), second(T2()) {}

  pair(const T1& a, const T2& b) : first(a), second(b) {}
};

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
  typedef Key key_type; // key类型
  typedef T data_type; // value类型
  typedef T mapped_type;
  // 这里key是const的
  typedef pair<const Key, T> value_type;
  typedef Compare key_compare;

  // 定义一个含有元素比较函数的类
  class value_compare : public binary_function<value_type, value_type, bool> {
    friend class map<Key, T, Compare, Alloc>;
  protected:
    Compare comp;
    value_compare(Compare c) : comp(c) {}
  public:
    bool operator()(const value_type& x, const value_type& y) const {
      return comp(x.first, y.first);
    }
  };
private:
  // 也是底层红黑树,和set几乎没啥区别
  typedef rb_tree<key_type, value_type, selectlst<value_type>, key_compare, Alloc> rep_type;
  rep_type t;
public:
  typedef typename rep_type::pointer pointer;
  typedef typename rep_type::const_pointer const_pointer;
  typedef typename rep_type::reference reference;
  typedef typename rep_type::const_reference const_reference;
  // 这里返回的是iterator
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;
  typedef typename rep_type::reverse_iterator reverse_iterator;
  typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
  typedef typename rep_type::size_type size_type;
  typedef typename rep_type::difference_type difference_type;

  map() : t(Compare()) {}
  explicit map(const Compare& comp) : t(comp) {}

  template <class InputIterator>
  map(InputIterator first, InputIterator last) : t(Compare()) {
    t.insert_unique(first, last);
  }

  template <class InputIterator>
  map(InputIterator first, InputIterator last, const Compare& comp) : t(comp) {
    t.insert_unique(first, last);
  }

  map(const map<Key, T, Compare Alloc>& x) : t(x.t) {}

  map<Key, T, Compare, Alloc>& operator=(const map<Key, T, Compare, Alloc>& x) {
    t = x.t;
    retrn *this;
  }

  // 全都是调用RB-tree的操作,它的操作和set几乎一样，是因为
  // 本来RB-tree的大多数操作都是针对key的，而value用到的很少
  key_compare key_comp() const {
    return t.key_comp();
  }
  value_compare value_comp() const { // map的key和value不同,比较规则也不同
    return value_compare(t.key_comp());
  }
  iterator begin() const {
    return t.begin();
  }
  iterator end() const {
    return t.end();
  }
  reverse_iterator rbegin() const {
    return t.rbegin();
  }
  reverse_iterator rend() const {
    return t.rend();
  }
  bool empty() const {
    return t.empty();
  }
  size_type size() const {
    return t.size();
  }
  size_type max_size() const {
    return t.max_size();
  }
  // 它比set多下标操作符
  T& operator[](const key_type& k) {
    // 这里的insert是干什么呢？
    // (insert(value_type(k, T()))).first:得到插入操作返回的pair的第一元素
    return (*((insert(value_type(k, T()))).first)).second;
  }
  void swap(set<Key, Compare, Alloc>& x) {
    t.swap(x.t);
  }
  // 它的返回值类型
  // RB-tree的insert_unique本来就有返回pair的版本
  pair<iterator, bool> insert(const value_type& x) {
    return t.insert_unique(x);
  }
  iterator insert(iterator position, const value_type& x){
    return t.unique(position, x);
  }
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last){
    t.insert_unique(first, last);
  }
  void erase(iterator position) {
    t.erase(position);
  }
  size_type erase(const key_type& x){
    return t.erase(x);
  }
  void erase(iterator first, iterator last){
    t.erase(first, last);
  }
  void clear() {
    t.clear();
  }

  iterator find(const key_type& x) const {
    return t.find(x);
  }
  size_type count(const key_type& x) const {
    return t.count(x);
  }
  iterator lower_bound(const key_type& x) const {
    return t.lower_bound(x);
  }
  const_iterator lower_bound(const key_type& x) const {
    return t.lower_bound(x);
  }
  iterator upper_bound(const key_type& x) const {
    return t.upper_bound(x);
  }
  const_iterator upper_bound(const key_type& x) const {
    return t.upper_bound(x);
  }
  pair<iterator, iterator> equal_range(const key_type& x) const {
    return t.equal_range(x);
  }
  pair<const_iterator, const_iterator> equal_range(const key_type& x) const {
    return t.equal_range(x);
  }

  friend bool operator==__STL_NULL_TMPL_ARGS(const map&, const map&);
  friend bool operator<__STL_NULL_TMPL_ARGS(const map&, const map&);

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

template <class Key, class Compare, class Alloc>
inline bool operator==(const map<Key, Compare, Alloc>& x, const map<Key, Compare, Alloc>& y) {
  // 就直接比较底层RB-tree是否相等
  return x.t == y.t;
}
template <class Key, class Compare, class Alloc>
inline bool operator< (const map<Key, Compare, Alloc>& x, const map<Key, Compare, Alloc>& y) {
  return x.t < y.t;
}


template <class Key, class Compare = less<Key>, class Alloc = alloc>
class multimap {
public:
  // 与map一样
  ...

  // 它调用的是可以添加重复数据的insert_equal()
  template <class InputIterator>
  multimap(InputIterator first, InputIterator last) : t(Compare()) {
    t.insert_equal(first, last);
  }
  template <class InputIterator>
  multimap(InputIterator first, InputIterator last, const Compare& comp) : t(comp) {
    t.insert_equal(first, last);
  }

  iterator insert(const value_type& x) {
    return t.insert_equal(x);
  }
  iterator insert(iterator position, const value_type& x) {
    return t.insert_equal(position, x);
  }

  template <class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    t.insert_equal(first, last);
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