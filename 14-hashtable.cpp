// HashFcn为了算出hashcode，可以得到每个元素应该放的位置， ExtractKey，为了萃取出key, EqualKey,你要告诉它什么是key相等或比较大小
template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc=alloc>
class hashtable {
public:
  typedef HashFcn hasher;
  typedef EqualKey key_equal;
  typedef size_t size_type;

private:
  hasher hash;
  key_equal equals;
  ExtractKey get_key;

  typedef __hashtable_node<Value> node;

  // 这个就是那个篮子
  vector<node*, Alloc> buckets;
  size_type num_elements;

public:
  size_type bucket_count() const { 
    return buckets.size();
  }
  ...
};

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator {
  ...
  // 它要有回到vector上去的能力
  node* cur; // 指向节点
  hashtable* ht; // 指向vector
};


template <class Value>
struct __hashtable_node {
  __hashtable_node* next;
  Value val;
};

// 泛化
template <class Key> struct hash { };

// 特化
__STL_TEMPLATE_NULL struct hash<char> {
  size_t operator()(char x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<short> {
  size_t operator()(short x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<unsigned short> {
  size_t operator()(unsigned short x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<int> {
  size_t operator()(int x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<unsigned int> {
  size_t operator()(unsigned int x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<long> {
  size_t operator()(long x) const { 
    return x;
  }
};
__STL_TEMPLATE_NULL struct hash<unsigned long> {
  size_t operator()(unsigned long x) const { 
    return x;
  }
};