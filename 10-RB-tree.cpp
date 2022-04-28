template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = alloc>
class rb_tree {
protected:
  typedef __rb_tree_node<Value> rb_tree_node;
  ...
public:
  typedef rb_tree_node* link_type;
  ...
protected:
  // RB-tree只以三个资料表现它自己
  size_type node_count; // rb-tree的大小(节点数)
  link_type header;
  Compare key_compare; // key的大小比较原则，这就是个 function object 
};