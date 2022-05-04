/**
 * @brief list的节点设计
 * 节点的数据结构
 * 可以看到的是：节点的大小不仅仅只有数据，还有向前向后的两个指针。
 * 很明显，这是一个双向链表
 * @version G2.9
 * @tparam T 
 */
template <class T>
struct __list_node {
  typedef void* void_pointer;
  void_pointer prev; // 向前指的指针
  void_pointer next; // 向后指的指针
  T data; // 数据
};

/**
 * @brief 
 * 
 * @version G4.9
 */
struct _List_node_base
{
  _List_node_base* _M_next;
  _List_node_base* _M_prev;
};

template<typename _Tp>
struct _List_node : public _List_node_base
{
  _Tp _M_data;
};

/**
 * @brief 
 * 
 * @tparam T 
 * @tparam Alloc 
 * @version G2.9
 */
template <class T, class Alloc = allocator>
class list {
protected:
  typedef __list_node<T> list_node;
  link_type node;
  // 专属空间配置器，一次配置一个节点
  typedef simple_alloc<list_node, Alloc> list_node_allocator;
  ...  
public:
  // 所有的容器都必须有一个iterator指向一个类,这个模板这里有三个参数，有些多余
  // 在G4.9版里就对它进行了优化
  typedef __list_iterator<T,T&,T*> iterator;
  typedef __list_iterator<T, Ref, Ptr> self;

  typedef bidirectional_iterator_tag iterator_category;
  typedef T value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef __list_node<T>* link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  list() {
    empty_initialized(); // 产生一个空链表
  }
  // constructor
  __list_iterator(link_type x) : node(x) { }
  __list_iterator() { }
  __list_iterator(const iterator& x) : node(x.node) { }

  bool operator==(const self& x) const {
    return node == x.node;
  }
  bool operator!=(const self& x) const {
    return node != x.node;
  }
  // 以下对迭代器取值(difference)，取的是节点的数据值
  reference operator*() const {
    return (*node).data;
  }

  // 以下是迭代器的成员存取(member access)运算的标准做法
  pointer operator->() const {
    return &(operator*());
  }

  // 对迭代器累加1，就是前进一个节点
  // 这就操作就是对指向节点的指针进行的操作
  self& operator++() {
    node = (link_type)((*node).next);
    return *this;
  }
  self operator++(int){
    self tmp = *this;
    ++*this;
    return tmp;
  }
  self operator--(){
    node = (link_type)((*node).prev);
    return *this;
  }
  self operator--(int){
    self tmp = *this;
    --*this;
    return tmp;
  }

  iterator begin() {
    return (link_type)((*node).next);
  }
  iterator end() {
    return node;
  }
  bool empty() const {
    return node->next == node;
  }
  size_type size() const {
    size_type result = 0;
    // 该函数返回两个迭代器之间的元素个数s
    distance(begin(), end(), result);
    return result;
  }
  // 取头节点的内容
  reference front() {
    return *begin();
  }
  reference back() {
    return *(--end());
  }

  // 对元素的操作
  void push_front(const T& x) {
    insert(begin(), x);
  }
  // push_back()也就是在末尾insert一个而已
  void push_back(const T& x) {
    insert(end(), x);
  }
  // 最简单的一个insert重载版本
  iterator insert(iterator position, const T& x) {
    // 先配置构造一个节点
    link_type tmp = create_node(x);
    // 调整指针
    tmp->next = position.node;
    tmp->prev = position.node->prev;
    (link_type(position.node->prev))->next = tmp;
    position.node->prev = tmp;
    return tmp;
  }
  // 移除迭代器哨兵位置所指节点
  iterator erase(iterator position) {
    // 标记一下上一个和下一个节点
    link_type next_node = link_type(position.node->next);
    link_type prev_node = link_type(position.node->prev);
    // 重新链接这节点
    prev_node->next = next_node;
    next_node->prev = prev_node;
    // 删除此节点
    destory_node(position.node);
    // 返回逻辑位置在删除位置的那个节点(就是删了的那个的下一个节点)
    return iterator(next_node);
  }

  // 删除头节点
  void pop_front() {
    erase(begin());
  }
  // 删除尾节点
  void pop_back() {
    // 能否用:erase(--end());来完成呢？
    iterator tmp = end();
    erase(--tmp);
  }

  // 将x接合于position之前，x必须不同于*this
  void splice(iterator position, last& x) {
    if(!x.empty())
      transfer(position, x.begin(), x.end()); 
  }
  // 将i所指的元素接合于position之前，position和i可指向同一个list
  void splice(iterator position, list&, iterator i) {
    iterator j = i;
    ++j;
    // 虽说是可以指向同一个list，但是i却不能是接合位置前的元素或接合位置处的元素
    if(position == i || position == j)
      return;
    transfer(position, i, j);
  }
  // 把[first, last)内的元素接合到position所指位置前，但position不能在position处
  void splice(iterator position, last&, iterator first, iterator last) {
    if(first != last)
      transfer(position, first, last);
  }

  void clear();

protected:
  // 配置一个节点并传回
  link_type get_node() {
    return list_node_allocator::allocate();
  }
  // 释放一个节点
  void put_node(link_type p) {
    list_node_allocator::deallocate(p);
  }

  // 配置并构造一个节点
  link_type create_node(const T& x) {
    link_type p = get_node();
    construct(&p->data, x);
    return p;
  }
  // 析构并释放一个节点
  void destory_node(link_type p) {
    destory(&p->data);
    put_node(p);
  }

  void empty_initialized() {
    node = get_node(); // 先开空间
    node->next = node; // 头尾都指向自己
    node->prev = node;
  }

  // 迁移操作，将[first, last)内的元素迁移到position之前，它可以帮助完成一些更加复杂的工作
  void transfer(iterator position, iterator first, iterator last) {
    if(position != last) {
      // 需要结合图来理解，最后不仅仅要迁移到的那个list完整，还要[first, last)原来所在的那个list完整
      (*(link_type((*last.node).prev))).next = position.node;
      (*(link_type((*first.node).prev))).next = last.node;
      (*(link_type((*position.node).prev))).next = first.node;
      link_type tmp = link_type((*position.node).prev);
      (*position.node).prev = (*last.node).prev;
      (*last.node).prev = (*first.node).prev;
      (*first.node).prev = tmp;
    }
  }
};

 // 清除所有节点
template <class T, class Alloc>
void list<T, Alloc>::clear() {
  // 这个node是应该表示链表的头指针
  link_type cur = (link_type)node->next; // 起始节点
  while(cur != node) { // 遍历每个节点
    link_type tmp = cur;
    cur = (link_type)cur->next; // 下一个节点
    destory_node(tmp); // 销毁节点
  }
  // 回复node原始状态
  node->next = node;
  node->prev = node;
}

// 将数值为value的所有元素移除
template <class T, class Alloc>
void list<T, Alloc>::remove(const T& value) {
  iterator first = begin();
  iterator last = end();
  while(first != last) {
    iterator next = first;
    +=next;
    if(*first == value)
      erase(first);
    first = next;
  }
}

// 移除数值相同的连续元素(连续而相同，会被删的只剩下一个,剩下的是第一个)
template <class T, class Alloc>
void list<T, Alloc>::unique() {
  iterator first = begin();
  iterator last = end();
  if (first == last)
    return;
  iterator next = first;
  while(++next != last) {
    if(*first == *next)
      erase(next);
    else
      first = next;
    next = first;
  }
}

// 在transfer的基础上完成的merge(),reverse(),sort()操作
template <class T, class Alloc>
void list<T, Alloc>::merge(list<T, Alloc>& x) { // 排完序的list放到调用这个方法的list里面
  iterator first1 = begin();
  iterator last1 = end();
  iterator first2 = x.begin();
  iterator last2 = x.end();

  // 两个list已经做过递增排序了
  while(first1 != last1 && first2 != last2) {
    if(*first2 < first1) {
      iterator next = first;
      transfer(first1, first2, ++next);
      first2 = next;
    }
    else ++first1;
  }
  if(first2 != last2)
    transfer(last1, first2, last2);
}

// reverse()将*this的内容逆向重置
template <class T, class Alloc>
void list<T, Alloc>::reverse() {
  // 空链表或者单独元素的连接表就不需要操作
  if(node->next == node || link_type(node->next)->next == node)
    return;
  iterator first = begin();
  +=first;
  while(first != end()) { // 一个一个的往前塞
    iterator old = first;
    ++first;
    transfer(begin(), old, first);
  }
}

// list自己的sort算法
template <class T, class Alloc>
void list<T, Alloc>::sort() {
  if(node->next == node || link_type(node->next)->next == node)
    return;
  
  // 创建一些list存放数据
  list<T, Alloc> carry;
  list<T, Alloc> counter[64];
  int fill = 0;
  while(!empty()) { // 判空
    carry.splice(carry.begin(), *this, begin());
    int i=0;
    // 这个不是弄出来一个然后和已经弄出来的那一堆比较，排好序保存到一个位置吗？为什么每次都要从0开始？
    // 奥，每次的counter[i]存放着第i次取出的那个数呢，只是counter[fill-1]里面存放所有取出的数
    // 经过测试得到：!counter[i].empty()这个也很关键，它是是否进入这个循环的判断，而i<fill是跳出循环的判断
    // counter里面按照0、2、4、8、16...个数据存放，这些都是有顺序且不重复的(指的不是字面值的重复，而是不是集合中重复取出或重复存放)
    while(i<fill && !counter[i].empty()) { // 最终的排序结果保存到carry中
      counter[i].merge(carry); // 排序完的放到counter[i]中
      carry.swap(counter[i++]); // 要把所有的都排序
    }
    carry.swap(counter[i]); // 把得到的结果放到counter[fill-1]中
    if(i == fill)
      ++fill;
  }
  for(int i=1; i<fill; ++i)
    counter[i].merge(counter[i-1]);
  // 这一步应该是this->swap(counter[fill-1])，把排序完的结果保存到原list中
  swap(counter[fill-1]);
}


/**
 * @brief 
 * 
 * @version G4.9
 * @tparam _Tp 
 * @tparam _Alloc 
 */
template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
class list : protected _List_base<_Tp, _Alloc> {
public:
  typedef _List_iterator<_Tp> iterator;
...
};

/**
 * @brief 这个是list的iterator
 * 把它设计成了一个类。
 * 
 * @version G2.9
 * @tparam T 
 * @tparam Ref 
 * @tparam Ptr 
 */
template<class T, class Ref, class Ptr>
struct __list_iterator {
  typedef __list_iterator<T, T&, T*> iterator;
  typedef __list_iterator<T, Ref, Ptr> self;

  // iterator必须提供这五种，以供算法使用
  typedef bidirectional_iterator_tag iterator_category; // 1.分类
  typedef T value_type; // 2.
  typedef Ptr pointer; // 3.
  typedef Ref reference; // 4.
  typedef __list_node<T>* link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type; // 5.

  link_type node; // 迭代器内部指向list节点的普通指针

  // construct
  __list_iterator(link_type x) : node(x) {

  }
  __list_iterator() {}
  __list_iterator(const iterator& x) : node(x.node) {}

  bool operator==(const self& x) const {
    return node == x.node;
  }
  bool operator!=(const self& x) const {
    return node != x.node;
  }
  // 以下对迭代器取值 (dereference) ,取的是节点的数据值
  reference operator*() const {
    // 理解的时候node就是当前的节点指针，平常咋取就咋取
    return (*node).data;
  }

  // 以下是迭代器的成员存取(member access)运算子的标准做法
  pointer operator->() const {
    return &(operator*());
  }

  // 对迭代器累加1，就是前进一个节点
  // 为了区分前置++和后置++，这里通过参数来设定
  // 后置需要加上参数，前置不需要加上参数(参数没有意义，只是为了重载)
  self& operator++(){
    // 就是把节点的next指针返回给迭代器的node指针
    node = (link_type)((*node).next);
    return *this;
  }
  // 这个是后置++
  self operator++(int) {
    // 1、记录原值
    // 在这里，需要先调用拷贝构造函数，创建tmp。
    // 而*this被当作了拷贝构造函数的参数，所以不会调用operator*
    self tmp = *this;
    // 2、进行操作
    // 这里调用的前置++的操作
    ++*this;
    // 3、返回原值
    // 返回的时候调用拷贝构造
    return tmp;
  }

  // 对迭代器递减1，就是后退一个节点
  // 这里返回reference，就可以连续进行前置--操作
  // 因为直接拿返回值进行操作
  self& operator--() {
    node = (link_type)((*node).prev);
    return *this;
  }
  // 这里返回value，返回value就不能连续进行两次后置--操作
  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }
};

template<typename _Tp>
struct _List_iterator
{
  // 这里就体现出了G4.9对它的优化
  typedef _Tp* pointer;
  typedef _Tp& reference;

};