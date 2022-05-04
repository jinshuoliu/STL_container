template <class T, class Alloc=alloc, size_t BufSiz=0> // 这里BufSiz默认为0，到__deque_buf_size函数中进行计算缓冲区大小
class deque {
public:
  typedef T value_type;
  typedef __deque_iterator<T, T&, T*, BufSiz> iterator;
protected:
  // 元素的指针的指针
  typedef pointer* map_pointer; // T**
protected:
  // 指向第一个节点的第一个位置
  iterator start;
  // 指向最后一个节点的最后一个位置的下一个位置
  iterator finish;
  // 指向map，map是连续空间
  map_pointer map;
  // map中指针的数量
  size_type map_size;
  // 专属空间配置器，每次配置一个元素大小
  typedef simple_alloc<value_type, Alloc> data_allocator;
  // 每次配置一个指针大小
  typedef simple_alloc<pointer, Alloc> map_allocator;
public:
  deque(int n, const value_type& value) : start(), finish(), map(0), map_size(0) {
    fill_initialize(n, value);
  }

  iterator begin() {
    return start;
  }

  iterator end() {
    return finish;
  }

  size_type size() const {
    return finish - start; // 这里就要去看看它的减法运算符了
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

  reference operator[](difference_type n) const {
    return *(*this + n);
  }

  bool operator==(const self& x) const {
    return cur == x.cur;
  }
  bool operator!=(const self& x) const {
    return !(*this == x);
  }
  bool operator<(const self& x) const {
    return (node == x.node) ? (cur < x.cur) : (node < x.node);
  }

  // push_back和push_front是有区别的：back要最后一个缓冲区里面至少有两个空间，而front只需要有一个就行了
  // 这就导致如果缓冲区空间不足的时候：back是把元素添加到最后一个位置，再添加一个缓冲区，而front是把元素添加到新增的缓冲区的最后面的位置上
  void push_back(const value_type& t) {
    if(finish.cur != finish.last - 1) { // 最后的这个缓冲区里面还有一个以上的空间(就是填充了这个元素，它还会有剩余，不会直接把这个缓冲区填满)
      construct(finish.cur, t); // 在备用空间上构建元素
      ++finish.cur; // 调整指向位置
    }
    else // 添了这个就满了，必须重新弄一块缓冲区
      push_back_aux(t);
  }
  void push_front(const value_type& t) {
    if(start.cur != start.first) { // 还有一个就行
      construct(start.cur - 1, t);
      --start.cur;
    }
    else
      push_front_aux(t);
  }
  void pop_back() {
    if(finish.cur != finish.first){
      --finish.cur;
      destory(finish.cur); // 析构最后的元素
    }
    else // 要释放的元素是缓冲区内最后一个元素了
      pop_back_aux();
  }
  void pop_front() {
    if(start.cur != start.alst - 1){
      destory(start.cur);
      ++start.cur;
    }
    else
      pop_front_aux();
  }

  // 删除一个指定元素
  iterator erase(iterator pos){
    iterator next = pos;
    +=next;
    difference_type index = pos - start; // 删除点之前的元素个数
    if(index < (size() >> 1) { // 如果删除点之前的元素个数比较少,就移动删除点之前的元素
      copy_backward(start, pos, next);
      pop_front();
    }
    else{
      copy(next, finish, pos);
      pop_back();
    }
    return start + index;
  }

protected: // 这个具体是protected还是private不清楚，但不应该是public
  // 什么时候map需要整治,实际的操作是由reallocate_map()完成的
  void reserve_map_at_back(size_type nodes_to_add = 1) {
    if(nodes_to_add + 1 > map_size - (finish.node - map)) // 这个不等式就是比较map尾部空闲空间与2的大小关系
      // 如果map尾端的节点空间不足，就换一个map，拷贝原来的到新配置的更大的空间，然后释放掉原来的
      reallocate_map(nodes_to_add, false);
  }
  void reserve_map_at_front(size_type nodes_to_add = 1) {
    if(nodes_to_add > start.node - map) // 比较头部空闲空间与1
      reallocate_map(nodes_to_add, true);
  }

  ...
};

// 这个是对缓冲区大小设定
inline size_t __deque_buf_size(size_t n, size_t sz) {
  return n != 0 ? n : (sz<512 ? size_t(512 / sz) :size_t(1));
}

/**
 * @brief 它负责产生并安排好deque的结构，并将元素的初值设定妥当
 * 
 * @tparam T 
 * @tparam Alloc 
 * @tparam BufSize 
 * @param n 
 * @param value 
 */
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::fill_initialize(size_type n, const value_type& value) {
  create_map_and_nodes(n); // 安排好deque的结构
  map_pointer cur;
  __STL_TRY {
    // 为每个节点的缓冲区设定初值
    for (cur = start.node; cur < finish.node; ++cur)
      uninitialized_fill(*cur, *cur + buffer_size(), value);
    // 最后一个节点的设定有些不同(因为尾端可能会有备用空间，不用设定初值)
    uninitialized_fill(finish.first, finish.cur, value);
  }
  catch(...) {
    ...
  }
}

/**
 * @brief 它负责产生并安排好deque的结构
 * 
 * @tparam T 
 * @tparam Alloc 
 * @tparam BufSize 
 * @param num_elements 
 */
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements) {
  // 需要的节点数=(元素个数/每个缓冲区可容纳的元素个数)+1
  // 如果刚好是整数，就多配一个节点
  size_type num_nodes = num_elements / buffer_size() + 1;

  // 一个map要管理多个节点，最少8个，最多为"所需节点数+2" +2是为了扩充时使用
  // 以下操作配置出一个"具有map_size个节点"的map
  map_size = max(initial_map_size(), num_nodes + 2);
  map = map_allocator::allocate(map_size);

  // 以下令nstart和nfinish指向map所拥有的全部节点的区段
  // 就是nstart和nfinish中间的区段就是有东西的区段，前后各有那么一两个空间
  map_pointer nstart = map + (map_size - num_nodes) / 2;
  map_pointer nfinish = nstart + num_nodes - 1; // 这里-1应该是因为前面+1了

  map_pointer cur;
  __STL_TRY {
    // 为map内的每个现用节点配置缓冲区， 所有缓冲区加起来就是deque的可用空间
    for (cur = nstart;cur <= nfinish; ++cur)
      *cur = allocate_node();
  }
  catch(...) {
    // "commit or rollback" 要么全成功，要么就不要了
    ...
  }

  // 为deque内的两个迭代器start和end设定正确的内容
  start.set_node(nstart);
  finish.set_node(nfinish);
  start.cur = start.first; // 因为cur和first是public的，所以可以在此操作
  finish.cur = finish.first + num_elements % buffer_size();

}

template <class T, class Alloc, size_t BufSize>
typename deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x) {
  difference_type index = pos - start; // 插入点之前的元素个数
  value_type x_copy = x;
  if(index < size() / 2) { // 前面元素少
    push_front(front()); // 在最前端加入第一个元素的同值元素
    iterator front1 = start; // 标记元素，然后进行元素搬移
    ++front1;
    iterator front2 = front1;
    ++front2;
    pos = start + index;
    iterator pos1 = pos;
    ++pos1;
    copy(front2, pos1, front1); // 元素搬移
  }
  else {
    push_back(back());
    iterator back1 = finish;
    --back1;
    iterator back2 = back1;
    --back2;
    pos = start + index;
    copy_backward(pos, back2, back1);
  }
  *pos = x_copy; // 插入点设置新值
  return pos;
}

/**
 * @brief 先配置一块新的缓冲区，安排元素位置再调整finish的状态
 * 
 * @tparam T 
 * @tparam Alloc 
 * @tparam BufSize 
 * @param t 
 */
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::push_back_aux(const value_type& t) {
  value_type t_copy = t;
  reserve_map_at_back(); // 肯能会重新换一个map(要符合一个条件)
  // 下面这才是常规操作
  *(finish.node+1) = allocate_node(); // 配置一个新缓冲区
  __STL_TRY {
    construct(finish.cur, t_copy); // 设定值，就是把要添加的值填到最后一个位置
    // 然后换一个新的缓冲区
    finish.set_node(finish.node + 1); // finish指向新节点
    finish.cur = finish.first; // 设定finish的状态
  }
  __STL_UNWIND(deallocate_node(*(finish.node + 1)));
}

template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::push_front_aux(const value_type& t) {
  value_type t_copy = t;
  reserve_map_at_front();
  *(start.node-1) = allocate_node();
  __STL_TRY {
    start.set_node(start.node - 1);
    start.cur = start.last - 1;
    construct(start.cur, t_copy);
  }
  catch(...) {
    // "commit or rollback"
    start.set_node(start.node + 1);
    start.cur = start.first;
    deallocate_node(*(start.node - 1));
    throw;
  }
}


template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add, bool add_at_front){ // 这个add_at_front参数用来区分back和front
  size_type old_num_nodes = finish.node - start.node + 1; // 原来的已经使用的节点数
  size_type new_num_nodes = old_num_nodes + nodes_to_add; // 之后的要被使用的节点数

  map_pointer new_nstart;
  if(map_size > 2 * new_num_nodes) { // 原来的存放map的空间还挺大，就是需要调整一下位置而已
    new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0); // 如果是因为push_front()导致的整治map，就多给前面一个节点
    if(new_nstart < start.node) // 要是新的start比旧的start位置靠前，就直接copy完事(从第一个开始复制)
      copy(start.node, finish.node + 1, new_nstart);
    else // 从最后一个开始复制
      copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
  }
  else { // 存放map的空间也不够了
    size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2; // 通常是扩二倍左右
    // 配置一块空间，配置给map使用
    map_pointer new_map = map_allocator::allocate(new_map_size);
    new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);

    copy(start.node, finish.node + 1, new_nstart);
    // 释放原map
    map_allocator::deallocate(map, map_size);
    // 重新设定map
    map = new_map;
    map_size = new_map_size;
  }

  // 重新设定迭代器start和finish
  start.set_node(new_nstart);
  finish.set_node(new_nstart + old_num_nodes - 1);
}

template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::pop_back_aux() {
  deallocate_node(finish.first); // 释放最后一个缓冲区
  finish.set_node(finish.node - 1); // 调整finish的状态，使它指向上一个缓冲区的最后一个元素
  finish.cur = finish.last - 1;
  destory(finish.cur); // 使该元素析构，这里有点问题把，不应该是先元素析构，再释放缓冲区，然后调整finish状态吗？
  // 这样做的原因：它是指向最后一个元素的下一个位置，所以析构此位置没错(还是有点小问题，它物理空间上并非连续的，这可能会造成它原来的元素没有释放，却释放了一个别的元素)
}

template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::pop_front_aux() {
  destory(start.cur);
  deallocate_node(start.first);
  start.set_node(start.node + 1);
  start.cur = start.first;
}

/**
 * @brief 清除整个deque，但是会保留一个缓冲区
 * 
 * @tparam T 
 * @tparam Alloc 
 * @tparam BufSize 
 */
template <class T, class Alloc, size_t BufSize>
void deque<T, Alloc, BufSize>::clear() {
  for (map_pointer node = start.node + 1; node < finish.node; ++ node) { // 针对头尾中间的那些缓冲区
    // 将缓冲区内所有元素析构
    destory(*node, *node + buffer_size());
    // 释放缓冲区内存
    data_allocator::deallocate(*node, buffer_size());
  }

  if(start.node != finish.node) { // 有头尾两个缓冲区
    destory(start.cur, start.alst);
    destory(finish.first, finish.cur);
    // 释放尾缓冲区，保留头缓冲区
    data_allocator::deallocate(finish.first, buffer_size());
  }
  else // 只有一个缓冲区
    destory(start.cur, finish.cur); // 只析构不释放
  finish = start; // 调整状态
}


template <class T, class Alloc, size_t BufSize>
deque<T, Alloc, BufSize>::iterator deque<T, Alloc, BufSize>::erase(iterator first, iterator last) {
  if(first == start && last == finish) { // 如果删除区间是整个deque
    clear();
    return finish;
  }
  else {
    difference_type n = last - first; // 要删除元素的长度
    difference_type elems_before = first - start; // 要删除的区间的前方的元素个数
    if(elems_before < (size() - n) / 2) { // 如果删除区间前面的元素少
      copy_backward(start, first, last); // 就是把前面那些元素一个个拽过来了
      iterator new_start = start + n; // 标记新起点
      destory(start, new_start;) // 将冗余的元素析构
      // 释放冗余的缓冲区
      for(map_pointer cur = start.node; cur < new_start.nod; +=cur)
        data_allocator::deallocate(*cur, buffer_size());
      start = new_start;
    }
    else {
      copy(last, finish, first);
      iterator new_finish = finish - n;
      destory(new_finish, finish);
      for(map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
        data_allocator::deallocate(*cur, buffer_size());
      finish = new_finish;
    }
    return start + elems_before;
  }
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