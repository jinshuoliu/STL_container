template <class T,class Sequence = vector<T>, class Compare = less<typename Sequence::value_type>>
class priority_queue {
public:
  typedef typename Sequence::value_type value_type;
  typedef typename Sequence::size_type size_type;
  typedef typename Sequence::reference reference;
  typedef typename Sequence::const_reference const_reference;
protected:
  Sequence c; // 底层容器vector
  Compare comp; // 元素比较大小的标准
public:
  priority_queue() : c() { }
  explicit priority_queue(const Compare& x) : c(), comp(x) { }

  template <class InputIterator>
  priority_queue(InputIterator first, InputIterator last, const Compare& x) : c(first, last), comp(x) {
    make_heap(c.begin, c.end(), comp);
  }
  template <class InputIterator>
  priority_queue(InputIterator first, InputIterator last) : c(first, last) {
    make_heap(c.begin(), c.end(), comp);
  }

  bool empty() const {
    return c.empty();
  }
  size_type size() const {
    return c.size();
  }
  const_reference top() const {
    return c.front();
  }
  void push(const value_type& x) {
    __STL_TRY {
      // push_heap要先把元素推进末尾才行，所以先调用底层容器的push_back将元素推入末端
      c.push_back(x);
      push_heap(c.begin(), c.end(), comp);
    }
    __STL_UNWIND(c.clear()); 
  }
  void pop() {
    __STL_TRY {
      pop_heap(c.begin(), c.end(), comp); // 同理
      c.pop_back();
    }
    __STL_UNWIND(c.clear());
  }
};