// 节点的基本结构
struct __slist_node_base{
  __slist_node_base* next;
};

// 节点结构
template <class T>
struct __slist_node : public __slist_node_base {
  T data;
};

// 全局函数，插入新节点到目标节点的后面
inline __slist_node_base* __slist_make_link(__slist_node_base* prev_node, __slist_node_base* new_node) {
  new_node->next = prev_node->next;
  prev_node->next = new_node;
  return new_node;
}

// slist的大小
inline size_t __slist_size(__slist_node_base* node) {
  size_t result = 0;
  for(; node != 0; node = node->next)
    ++result;
  return result;
}

#include <stddef.h> // 引入ptrdiff_t类型
struct __slist_iterator_base {
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef forward_iterator_tag iterator_category;

  __slist_node_base* node;

  __slist_iterator_base(__slist_node_base* x) : node(x) { }

  void incr() { // 前进一个
    node = node->next;
  }
  bool operator==(const __slist_iterator_base& x) const {
    return node == x.node;
  }
  bool operator!=(const __slist_iterator_base& x) const {
    return node != x.node;
  }
};

template <class T, class Ref, class Ptr>
struct __slist_iterator : public __slist_iterator_base {
  typedef __slist_iterator<T, T&, T*> iterator;
  typedef __slist_iterator<T, const T&, const T*> const_iterator;
  typedef __slist_iterator<T, Ref, Ptr> self;

  typedef T value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef __slist_node<T> list_node;

  __slist_iterator(list_node* x) : __slist_iterator_base(x) { }
  // 调用slist<T>::end()时会造成__slist_iterator_base(0)
  __slist_iterator() : __slist_iterator_base(0) { }
  __slist_iterator(const iterator& x) : __slist_iterator_base(x.node) { }

  reference operator*() const {
    return ((list_node*) node)->data;
  }
  pointer operator->() const {
    return &(operator*());
  }

  // 没有--
  self& operator++() {
    incr();
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    incr();
    return tmp;
  }
};

template <class T, class Alloc=alloc>
class slist {
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef __slist_iterator<T, T&, T*> iterator;
  typedef __slist_iterator<T, const T&, const T*> const_iterator;

private:
  typedef __slist_node<T> list_node;
  typedef __slist_node_base list_node_base;
  typedef __slist_iterator_base iterator_base;
  typedef simple_alloc<list_node, Alloc> list_node_allocator;

  struct list_node* create_node(const value_type& x) {
    list_node* node = list_node_allocator::allocate(); // 配置空间，这里用的是list的allocate()
    __STL_TRY {
      construct(&node->data, x); // 构造元素
      node->next = 0;
    }
    __STL_UNWIND(list_node_allocator::deallocate(node));
    return node;
  }

  static void destory_node(list_node* node) {
    destory(&node->data); // 将元素析构
    list_node_allocator::deallocate(node); // 释放空间
  }

private:
  list_node_base head; // 头部，没数据

public:
  slist() {
    head.next = 0;
  }
  ~slist() {
    clear();
  }

public:
  iterator begin() {
    return iterator((list_node*)head.next);
  }
  iterator end() {
    return iterator(0);
  }
  size_type size() const {
    return __slist_size(head.next);
  }
  bool empty() const {
    return head.next == 0;
  }

  // 交换两个slist,就让他们的head指向互换
  void swap(slist& L) {
    list_node_base* tmp = head.next;
    head.next = L.head.next;
    L.head.next = tmp;
  }

  reference front() { // 取头部元素
    return ((list_node*)head.next)->data;
  }

  void push_front(const value_type& x) {
    __slist_make_link(&head, create_node(x));
  }

  void pop_front() { // 就是简单的切换指向，然后释放那个空间
    list_node* node = (list_node*)head.next;
    head.next = node->next;
    destory_node(node);
  }
};