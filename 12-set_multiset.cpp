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
  template <class T>
  struct identity : public unary_function<T, T> {
    const T& operator()(const T& x) const {
      return x;
    }
  }
  // 里面含有一个红黑树
  typedef rb_tree<key_type, value_type, identity<value_type>, key_compare, Alloc> rep_type;
  rep_type t;
public:
  typedef typename rep_type::const_pointer pointer;
  typedef typename rep_type::const_pointer const_pointer;
  typedef typename rep_type::const_reference reference;
  typedef typename rep_type::const_reference const_reference;
  // 这里拿的是const_iterator,也就导致了不可以改变元素的值,不允许用户在任意处进行写入操作  
  typedef typename rep_type::const_iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;
  typedef typename rep_type::const_reverse_iterator reverse_iterator;
  typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
  typedef typename rep_type::size_type size_type;
  typedef typename rep_type::difference_type difference_type;
  
  // set的所有底层操作都是用t来完成的，可以把set当作红黑树的适配器
  // 就像是stack、queue与deque的关系
  set() :t(Compare()) {}
  explicit set(const Compare& comp) : t(comp) {}

  // 注意set只能调用RB-tree的insert_unique
  template <class InputIterator>
  set(InputIterator first, InputIterator last) : t(Compare()) {
    t.insert_unique(first, last);
  }

  template <class InputIterator>
  set(InputIterator first, InputIterator last, const Compare& comp) : t(comp) {
    t.insert_unique(first, last);
  }

  set(const set<Key, Compare, Alloc>& x) : t(x.t) {}
  set<Key, Compare, Alloc>& operator=(const set<Key, Compare, Alloc>& x) {
    t = x.t;
    return *this;
  }

  // 下面操作完全调用RB-tree的操作
  key_compare key_comp() const {
    return t.key_comp();
  }
  value_compare value_comp() const {
    return t.key_comp();
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
  void swap(set<Key, Compare, Alloc>& x) {
    t.swap(x.t);
  }

  typedef pair<iterator, bool> pair_iterator_bool;
  pair<iterator, bool> insert(const value_type& x){
    pair<typename rep_type::iterator, bool> p = t.insert_unqiue(x);
    return pair<iterator, bool>(p.first, p.second);
  }
  iterator insert(iterator position, const value_type& x){ // 它还有指定位置的插入吗？
    typedef typename rep_type::iterator rep_iterator;
    return t.insert_unique((rep_iterator&)position, x);
  }
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last){
    t.insert_unique(first, last);
  }
  void erase(iterator position){
    typedef typename rep_type::iterator rep_iterator;
    t.erase((rep_iterator&)position);
  }
  size_type erase(const key_type& x){
    return t.erase(x);
  }
  void erase(iterator first, iterator last){
    typedef typename rep_type::iterator rep_iterator;
    t.erase((rep_iterator&)first, (rep_iterator&)last);
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
  iterator upper_bound(const key_type& x) const {
    return t.upper_bound(x);
  } 
  pair<iterator, iterator> equal_range(const key_type& x) const {
    return t.equal_range(x);
  }
  
  friend bool operator== __STL_NULL_TMPL_ARGS (const set&, const set&);
  friend bool operator< __STL_NULL_TMPL_ARGS (const set&, const set&);

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

template <class Key, class Compare, class Alloc>
inline bool operator==(const set<Key, Compare, Alloc>& x, const set<Key, Compare, Alloc>& y) {
  // 就直接比较底层RB-tree是否相等
  return x.t == y.t;
}
template <class Key, class Compare, class Alloc>
inline bool operator< (const set<Key, Compare, Alloc>& x, const set<Key, Compare, Alloc>& y) {
  return x.t < y.t;
}


/**
 * @brief 
 * 它与multiset除了下面的内容全部一样
 * @tparam Key 
 * @tparam Compare 
 * @tparam Alloc 
 */
template <class Key, class Compare = less<Key>, class Alloc = alloc>
class multiset {
public:
  // 与set一样
  ...

  // 它调用的是可以添加重复数据的insert_equal()
  template <class InputIterator>
  multiset(InputIterator first, InputIterator last) : t(Compare()) {
    t.insert_equal(first, last);
  }
  template <class InputIterator>
  multiset(InputIterator first, InputIterator last, const Compare& comp) : t(comp) {
    t.insert_equal(first, last);
  }

  iterator insert(const value_type& x) {
    return t.insert_equal(x);
  }
  iterator insert(iterator position, const value_type& x) {
    typedef typename rep_type::iterator rep_iterator;
    return t.insert_equal((rep_iterator&)position, x);
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