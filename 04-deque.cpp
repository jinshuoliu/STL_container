template <class T, class Alloc=alloc, size_t BufSiz=0> // 这里BufSiz默认为0，到__deque_buf_size函数中进行计算缓冲区大小
class deque {
public:
  typedef T value_type;
  typedef __deque_iterator<T, T&, T*, BufSiz> iterator;
protected:
  typedef pointer* map_pointer; // T**
protected:
  iterator start;
  iterator finish;
  map_pointer map;
  size_type map_size;
public:
  iterator begin() {
    return start;
  }

  iterator end() {
    return finish;
  }

  size_type size() const {
    return finish - start;
  }

  reference operator[] (size_type n) {
    return start[difference_type(n)];
  }

  reference front() {
    return *start;
  }

  reference back() {
    // 因为finish指向的是最后一个元素的下一个元素，所以要向前回滚一个
    iterator tmp = finish;
    --tmp;
    return *tmp;
  }

  bool empty() const {
    return finish == start;
  }

   // 在position处安插一个元素，其值为x
  iterator insert(iterator position, const value_type& x) {
    // 先排除以下首位添加的情况
    if(position.cur == start.cur) {
      push_front(x);
      return start;
    }
    else if (position.cur == finish.cur) {
      push_back(x);
      iterator tmp = finish;
      --tmp;
      return tmp;
    }
    else {
      return insert_aux(position, x);
    }
  }

  reference operator*() const {
    // cur才是真正的那个值
    return *cur;
  }

  pointer operator->() const {
    return &(operator*());
  }

  // deque模拟连续空间
  // 两个iterator之间的距离如何计算
  //  两个iterator之间的buffers的总长度+itr到其他buffer末尾的长度+x到其buffer起始的长度
  difference_type operator-(const self& x) const {
    return difference_type(buffer_size()) * (node - x.node - 1) + (cur - first) + (x.last - x.cur);
  }

  // 移动一个位置
  self& operator++() {
    // 这里为什么可以直接++再判断是否是last呢？
    // 因为last是指向最后一个空间的下一个空间的特性
    ++cur;
    if (cur == last) {
      set_node(node + 1);
      cur = first;
    }
    return *this;
  }

  self operator++(int){
    self tmp = *this;
    // 后++调用前++
    ++*this;
    return tmp;
  }

  self& operator--() {
    if(cur == first) {
      set_node(node-1);
      cur = last;
    }
    --cur;
    return *this;
  }

  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  void set_node(map_pointer new_node) {
    node = new_node;
    first = *new_node;
    last = first + difference_type(buffer_size());
  }


  // 移动多个位置
  self& operator+=(difference_type n) {
    difference_type offset = n + (cur - first);
    if(offset >= 0 && offset < difference_type(buffer_size()))
      // 目标位置在同一个缓冲区内
      cur += n;
    else {
      // 目标位置不在同一个缓冲区内
      // 看看跳跃了多少个缓冲区(这里是因为可能+=负数)
      difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) : -difference_type((-offset - 1) / buffer_size()) - 1;
      // 切换到正确的节点(缓冲区)
      set_node(node + node_offset);
      // 切换到正确的元素
      cur = first + (offset - node_offset * difference_type(buffer_size()));
    }
    return *this;
  }

  // +调用的是+=
  self operator+(difference_type n) const {
    self tmp = *this;
    return tmp += n;
  }

  // -=就是+=(负数)
  self& operator-=(difference_type n) {
    return * this+= -n;
  }

  self operator-(difference_type n) {
    self tmp = *this;
    return tmp -= n;
  }

  self operator[](difference_type n) const {
    return *(*this + n);
  }
  ...
};

// 这个是对缓冲区大小设定
inline size_t __deque_buf_size(size_t n, size_t sz) {
  return n != 0 ? n : (sz<512 ? size_t(512 / sz) :size_t(1));
}

/**
 * @brief deque的迭代器
 * 
 * @tparam T 
 * @tparam Ref 
 * @tparam Ptr 
 * @tparam BufSiz 
 */
template <class T, class Ref, class Ptr, size_t BufSiz>
struct __deque_iterator { // 没有继承 std::iterator
  typedef __deque_iterator<T, T&, T*, BufSiz> iterator;
  typedef __deque_iterator<T, const T&, const T*, BufSiz> const_iterator;
  static size_t buffer_size() { // 用来决定缓冲区大小的函数
    return __deque_buf_size(BufSiz, sizeof(T));
  }

  // 因为没有继承 std::iterator 所以需要自己写5个必要的迭代器相应类型
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T** map_pointer;

  typedef __deque_iterator self;

  // 保持与容器的联结
  T* cur; // 此迭代器指向缓冲区的现行元素
  T* first; // 指向头
  T* last; // 指向尾
  map_pointer node; // 指向管控中心(就是那个存放所有缓冲区指针的地方)
  ...
};

template <class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x) {
  difference_type index = pos - start; // 插入点之前的元素个数
  value_type x_copy = x;
  if(index < size() / 2) { // 前面元素少
    push_front(front()); // 在最前端加入第一个元素的同值元素
    ...
    copy(front2, pos1, front1); // 元素搬移
  }
  else {
    push_back(back());
    ...
    copy_backward(pos, back2, back1);
  }
  *pos = x_copy; // 插入点设置新值
  return pos;
}