typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false; // 红色为0
const __rb_tree_color_type __rb_tree_black = true; // 黑色为1

/**
 * @brief 节点的父类
 * 
 */
struct __rb_tree_node_base {
  typedef __rb_tree_color_type color_type;
  typedef __rb_tree_node_base* base_ptr;

  // 节点的颜色，非黑即红
  color_type color;
  // 必须知道父子节点
  base_ptr parent;
  base_ptr left;
  base_ptr right;

  // 找最大最小就是找最左子树的叶子和最右子树的叶子
  static base_ptr minimum(base_ptr x) {
    while(x->left != 0)
      x = x->left;
    return x;
  }
  static base_ptr maximum(base_ptr x) {
    while(x->right != 0)
      x = x->right;
    return x;
  }
};

/**
 * @brief 节点类
 * 继承自base类
 * 
 * @tparam Value 
 */
template <class Value>
struct __rb_tree_node : public __rb_tree_node_base {
  typedef __rb_tree_node<Value>* link_type;
  Value value_field; // 节点值
};

/**
 * @brief base迭代器
 * 
 */
struct __rb_tree_base_iterator {
  typedef __rb_tree_node_base::base_ptr base_ptr;
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;

  base_ptr node; //用来与容器之间产生连结关系

  // 可实现于operator++内
  void increment() {
    if(node->right != 0){ // 状况1：它有右节点，那么++就应该是它右子树的最左下的叶子
      node = node->right;
      while(node->left != 0)
        node = node->left;
    }
    else { // 它没有右节点
      base_ptr y = node->parent; // 就需要回溯了
      while(node == y->right) { // 跳出node等于父节点的右节点的情况
        node = y;
        y = y->parent;
      }
      if(node->right != y)
        node = y; // 正常情况应该指向它，但是如果是没有下一节点的情况，就不执行了
    }
  }

  void decrement() {
    if(node->color == __rb_tree_red && node->parent->parent == node) {
      node = node->right; // 这是node为header的情况
    }
    else if (node->left != 0) { // 普通情况
      base_ptr y = node->left;
      while(y->right != 0)
        y = y->right;
      node = y;
    }
    else {
      base_ptr y = node->parent;
      while(node == y->left) {
        node = y;
        y = y->parent;
      }
      node = y;
    }
  }
};

template <class Value, class Ref, class Ptr>
struct __rb_tree_iterator : public __rb_tree_base_iterator {
  typedef Value value_type;
  typedef Ref reference;
  typedef Ptr pointer;
  typedef __rb_tree_iterator<Value, Value&, Value*> iterator;
  typedef __rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
  typedef __rb_tree_iterator<Value, Ref, Ptr> self;
  typedef __rb_tree_node<Value>* link_type;

  __rb_tree_iterator() { }
  __rb_tree_iterator(link_type x) { node = x; }
  __rb_tree_iterator(const iterator& it) { node = it.node; }

  reference operator*() const {
    return link_type(node)->value_field;
  }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
  pointer operator->() const {
    return &(operator*());
  }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */

  self& operator++() {
    increment();
    return *this;
  }

  self operator++(int) {
    self tmp = *this;
    increment();
    return tmp;
  }

  self& operator--() {
    decrement();
    return *this;
  }

  self operator--(int) {
    self tmp = *this;
    decrement();
    return tmp;
  }

};


template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = alloc>
class rb_tree {
protected:
  typedef void* void_pointer;
  typedef __rb_tree_node_base* base_ptr;
  typedef __rb_tree_node<Value> rb_tree_node;
  typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator; // RB-tree的专属空间配置器，每次可配置一个节点
  typedef __rb_tree_color_type color_type;
public:
  typedef Key key_type;
  typedef Value value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef rb_tree_node* link_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
protected:
  link_type get_node() {
    return rb_tree_node_allocator::allocate();
  }
  void put_node(link_type p) {
    rb_tree_node_allocator::deallocate(p);
  }

  link_type create_node(const value_type& x) {
    link_type tmp = get_node(); // 配置空间
    __STL_TRY {
      // 这个value_field是定义在__rb_tree_node中的节点值
      construct(&tmp->value_field, x); // 构造内容
    }
    __STL_UNWIND(put_node(tmp));
    return tmp;
  }
  link_type clone_node(link_type x) { // 复制节点
    // 就是把那个节点的节点值取出来构造一个新节点
    link_type tmp = create_node(x->value_field);
    // 颜色和复制的那个的颜色一样
    tmp->color = x->color;
    tmp->left = 0;
    tmp->right = 0;
    return tmp;
  }
  void destory_node(link_type p) {
    destory(&p->value_field); // 析构内容
    put_node(p); // 释放内存
  }
protected:
  // RB-tree只以三个资料表现它自己
  size_type node_count; // rb-tree的大小(节点数)
  link_type header;
  Compare key_compare; // key的大小比较原则，这就是个 function object
  
  // 以下三个函数用来方便取得header的成员,返回reference，可以做左值
  link_type& root() const {
    return (link_type&)header->parent;
  }
  link_type& leftmost() const {
    return (link_type&)header->left;
  }
  link_type& rightmost() const {
    return (link_type&)header->right;
  }

  // 以下六个函数用来方便取得节点x的成员
  static link_type& left(link_type x) {
    return (link_type&)(x->left);
  }
  static link_type& right(link_type x) {
    return (link_type&)(x->right);
  }
  static link_type& parent(link_type x) {
    return (link_type&)(x->parent);
  }
  static reference value(link_type x) {
    return x->value_field;
  }
  static const Key& key(link_type x) {
    return KeyOfValue()(value(x));
  }
  static color_type& color(link_type x) {
    return (color_type&)(x->color);
  }


  // 以下六个函数用来方便取得节点x的成员
  static link_type& left(base_ptr x) {
    return (link_type&)(x->left);
  }
  static link_type& right(base_ptr x) {
    return (link_type&)(x->right);
  }
  static link_type& parent(base_ptr x) {
    return (link_type&)(x->parent);
  }
  static reference value(base_ptr x) {
    return ((link_type)x)->value_field;
  }
  static const Key& key(base_ptr x) {
    return KeyOfValue()(value(link_type(x)));
  }
  static color_type& color(base_ptr x) {
    return (color_type&)(link_type(x)->color);
  }

  // 求极大值和极小值。node class 有实现此功能
  static link_type minimum(link_type x) {
    return (link_type)__rb_tree_node_base::minimum(x);
  }
  static lilnk_type maximum(link_type x) {
    return (link_type)__rb_tree_node_base::maximum(x);
  }

public:
  typedef __rb_tree_iterator<value_type, reference, pointer> iterator;

private:
  iterator __insert(base_ptr x, base_ptr y, const value_type& v);
  link_type __copy(link_type x, link_type p);
  void __erase(link_type x);
  void init() {
    header = get_node(); // 产生一个节点空间，令header指向它
    color(header) = __rb_tree_red; // 令header为红色，用来区分header和root，将用于operator++ 之中
    root() = 0;
    leftmost() = header; // 令header的左子节点为自己
    rightmost() = header; // 令header的右子节点为自己
  }

public:
  rb_tree(const Compare& comp = Compare()) : node_count(0), key_compare(comp){
    init();
  }

  ~rb_tree() {
    clear();
    put_node(header);
  }

  rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);

public:
  Compare key_comp() const {
    return key_compare;
  }
  iterator begin() {
    // 起点也就是最小节点处
    return leftmost();
  }
  iterator end() {
    // 终点为header指向的地方
    return header;
  }
  bool empty() const {
    return node_count == 0;
  }
  size_type size() const {
    return node_count;
  }
  size_type max_size() const {
    // 它的意思是返回平台所允许的最大数
    return size_type(-1);
  }

public:
  // 将x插入到RB-tree中(不允许重复)
  pair<iterator, bool> insert_unique(const value_type& x);
  // 将x插入到RB-tree中(允许重复)
  iterator insert_equal(const value_type& x);

  typename iterator find(const Key& k);

};

/*
插入元素
*/

/**
 * @brief 允许重复插入
 * 
 * @tparam Key 
 * @tparam Value 
 * @tparam KeyOfValue 
 * @tparam Compare 
 * @tparam Alloc 
 * @param v 
 * @return rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 返回一个RB-tree迭代器，指向新增节点
 */
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
  rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const Value& v) {
    link_type y = header;
    link_type x = root(); // 从跟节点开始向下找
    while(x!=0) {
      y = x;
      x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x); // 小于向左，大于向右
    }
    // x是插入点位置，y是插入点的父节点，v是新值
    return __insert(x, y, v);
}

/**
 * @brief 无重复插入
 * 
 * @tparam Key 
 * @tparam Value 
 * @tparam KeyOfValue 
 * @tparam Compare 
 * @tparam Alloc 
 * @param v 
 * @return pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool> 
 */
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool> 
  rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_unique(const Value& v) {
    link_type y = header;
    link_type x = root();
    bool comp = true;
    while(x != 0) {
      y = x;
      // 判断v的值是否小于当前节点的值
      comp = key_compare(KeyOfValue()(v), key(x));
      // 小于向左，等于/大于向右
      x = comp ? left(x) : right();
    }
    // y指向插入节点的父节点，令迭代器j指向它
    iterator j = iterator(y);
    if(comp) // comp为true表示插入点是y的左节点
      if(j == begin()) // 如果插入点是最左节点
        // 这里如果是最左边是可以直接插入，就不需要考虑之前有过因为等于而向右的情况了
        // 如果不是最左边它就需要考虑相等的情况了
        return pair<iterator, bool>(__insert(x, y, v), true);
      else
        --j; //  
        /* 
        这里弄不明白，直接--它指向哪里呢？
        1.不知想这颗树了，那么下面的判断它也是不可以的，它就直接不可以插入到树里面了
        这样是对的吗？他虽然可能和它前面的某个父节点相同，但并不绝对
        2.它重新指向这个父节点的父节点，这样它就可以进入到下面的判断，就可以正常进行 
        */
    if(key_compare(key(j.node), KeyOfValue()(v))) // 父节点的值是否小于新值，小于就插入到右边
      return pair<iterator, bool>(__insert(x, y, v), true);
    return pair<iterator, bool>(j, false);
}


/**
 * @brief 真正的插入
 * 
 * @tparam Key 
 * @tparam Value 
 * @tparam KeyOfValue 
 * @tparam Compare 
 * @tparam Alloc 
 * @param x_ 
 * @param y_ 
 * @param v 
 * @return rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
 */
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
  rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::__insert(base_ptr x_, base_ptr y_, const Value& v) {
    // 参数x_为新值插入点，y_为插入点的父节点，v为新值
    link_type x = (link_type)x_;
    link_type y = (link_type)y_;
    link_type z;

    if(y == header || x != 0 || key_compare(KeyOfValue()(v), key(y))) {
      // 三种情况：y是header，x不等于0(一个节点 等于0意味着什么？)，插入到左边
      z = create_node(v);
      left(y) = z; // 把新增的插入到左边
      if(y == header) { // y是header会导致什么情况
        root() = z; // 维护root()和rightmost()
        rightmost() = z;
      }
      else if (y == leftmost()) // 当父节点是最小的那个呢
        leftmost() = z; // 维护最小节点
    }
    else { // 要插入到右边的情况
      z = create_node(v);
      right(y) = z;
      if(y == rightmost())
        rightmost() == z; // 维护最大节点
    }
    parent(z) = y;
    left(z) = 0;
    right(z) = 0;

    __rb_tree_rebalance(z, header->parent); // 调整颜色
    ++node_count; // 节点数累加
    return iterator(z); // 返回一个指向新增节点的迭代器 
}

inline void __rb_tree_rotate_left(__rb_tree_node_base* x, __rb_tree_node_base*& root);
inline void __rb_tree_rotate_right(__rb_tree_node_base* x, __rb_tree_node_base*& root);
/**
 * @brief Construct a new rb tree rebalance object
 * 完成插入操作后，需要进行一次调整操作，将数的状态调整到符合RB-tree的要求
 * @param x 
 * @param root 
 */
inline void __rb_tree_rebalance(__rb_tree_node_base* x, __rb_tree_node_base*& root) {
  x->color = __rb_tree_red; // 新节点必为红色
  // 这就是自上而下改变颜色的过程，逻辑和之前的插入节点的四种情况一一对应
  while(x!=root && x->parent->color==__rb_tree_red) { // 父节点必为红
    if(x->parent == x->parent->parent->left){ // 父节点为祖父节点的左子节点
      __rb_tree_node_base* y = x->parent->parent->right; // 令y为伯父节点
      if(y && y->color==__rb_tree_red) { // 伯父节点存在，且为红
        // 改父节点、伯父节点为黑
        x->parent->color = __rb_tree_black;
        y->color = __rb_tree_black;
        // 改祖父节点为红
        x->parent->parent->color = __rb_tree_red;
        x = x->parent->parent;
      }
      else { // 无伯父节点或伯父节点为黑
        if(x==x->parent->right) { // 如果新节点为父节点的右子节点
          x = x->parent;
          __rb_tree_rotate_left(x, root); // 左旋，第一参数为左旋点
        }
        x->parent->color = __rb_tree_black;
        x->parent->parent->color = __rb_tree_red;
        __rb_tree_rotate_right(x->parent->parent, root);
      }
    }
    else { // 父节点为祖父节点的右子节点
      __rb_tree_node_base* y = x->parent->parent->left;
      if(y && y->color == __rb_tree_red) { // 有伯父节点，且为红
        x->parent->color = __rb_tree_black;
        y->color = __rb_tree_black;
        x->parent->parent->color = __rb_tree_red;
        x = x->parent->parent;
      }
      else {
        if(x==x->parent->left){
          x = x->parent;
          __rb_tree_rotate_right(x, root);
        }
        x->parent->color = __rb_tree_black;
        x->parent->parent->color = __rb_tree_red;
        __rb_tree_rotate_left(x->parent->parent, root);
      }
    }
  }
  root->color = __rb_tree_black; // 根节点永远为黑
}

inline void __rb_tree_rotate_left(__rb_tree_node_base* x, __rb_tree_node_base*& root) {
  // x为旋转点
  __rb_tree_node_base* y = x->right; // y为旋转点的右子节点
  x->right = y->left;
  if(y->left != 0)
    y->left->parent = x;
  y->parent = x->parent;

  // 令y完全替代x的地位
  if(x == root)
    root = y;
  else if(x == x->parent->left)
    x->parent->left = y;
  else
    x->parent->right = y;
  x->left = x;
  x->parent = y;
}

inline void __rb_tree_rotate_right(__rb_tree_node_base* x, __rb_tree_node_base*& root) {
  // x为旋转点
  __rb_tree_node_base* y = x->left; // y为旋转点的右子节点
  x->left = y->right;
  if(y->right != 0)
    y->right->parent = x;
  y->parent = x->parent;

  // 令y完全替代x的地位
  if(x == root)
    root = y;
  else if(x == x->parent->right)
    x->parent->right = y;
  else
    x->parent->left = y;
  x->right = x;
  x->parent = y;
}

/*
搜索元素
*/
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::find(const Key& k) {
  link_type y = header;
  link_type x = root();

  while(x != 0)
    if(!key_compare(key(x), k))
      y = x, x = left(x);
    else
      x = right(x);
  
  iterator j = iterator(y);
  return (j == end() || key_compare(k, key(j.node)))? end() : j;
}