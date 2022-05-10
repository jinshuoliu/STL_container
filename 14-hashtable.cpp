/**
 * @brief hash table 的节点的定义
 * 
 * @tparam Value 
 */
template <class Value>
struct __hashtable_node {
  __hashtable_ndoe& next;
  Value val;
};


template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator {
  typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> hashtable;
  typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> iterator;
  typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> const_iterator;
  typedef __hashtable_node<Value> node;

  // 迭代器必要的五个
  typedef forward_iterator_tag iterator_category;
  typedef Value value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef Value& reference;
  typedef Value* pointer;

  node* cur; // 迭代器目前所指向的节点
  hashtable* ht; // 保持对容器的连结关系

  __hashtable_iterator(node* n, hashtable* tab) : cur(n), ht(tab) {}
  __hashtable_iterator() {}

  reference operator*() const {
    return cur->val;
  }
  pointer operator->() const {
    return &(operator*());
  }
  iterator& operator++();
  iterator operator++(int);

  bool operator==(const iterator& it) const {
    return cur==it.cur;
  }
  bool operator!=(const iterator& it) const {
    return cur!=it.cur;
  }
};

/**
 * @brief ++操作，需要等看完hashtable才清楚
 * 
 * @tparam V 
 * @tparam K 
 * @tparam HF 
 * @tparam ExK 
 * @tparam EqK 
 * @tparam A 
 * @return __hashtable_iterator<V, K, HF, ExK, EqK, A>& 
 */
template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>& __hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++() {
  const node* old = cur;
  cur = cur->next; // 如果存在，就结束了
  if(!cur) { // 它不存在下一个了，就需要进入下一个bucket了
    // bkt_num():用于计算bucket的位置，它调用hash function取得一个可执行取模运算的值
    size_type bucket = ht->bkt_num(old->val);
    while(!cur && ++bucket<ht->buckets.size())
      cur = ht->buckets[bucket];
  }
  return *this;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_iterator<V, K, HF, ExK, EqK, A> __hashtable_iterator<V, K, HF, ExK, EqK, A>::operator(int) {
  iterator tmp = *this;
  ++*this;
  return tmp;
}

// HashFcn为了算出hashcode，可以得到每个元素应该放的位置， ExtractKey，为了萃取出key, EqualKey,你要告诉它什么是key相等或比较大小
/**
 * @brief 
 * 
 * @tparam Value 节点的实值型别
 * @tparam Key 节点的键值型别
 * @tparam HashFcn hash function的函数型别
 * @tparam ExtractKey 从节点中取出键值的方法(函数或仿函数)
 * @tparam EqualKey 判断键值是否相同的方法(函数或仿函数)
 * @tparam Alloc 空间配置器
 */
template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey, class Alloc=alloc>
class hashtable {
public:
  typedef HashFcn hasher; // 为template型别参数重新定义一个名称
  typedef EqualKey key_equal;
  typedef size_t size_type;

private:
  // 以下三个都是function objects
  // <stl_hash_fun.h>中定义有数个标准型别(如int，c-style string等)的hasher
  hasher hash;
  key_equal equals;
  ExtractKey get_key;

  typedef __hashtable_node<Value> node;
  // 专属的节点配置器
  typedef simple_alloc<node, Alloc> node_allocator;

  // 节点配置函数
  node* new_node(const value_type& obj){
    node* n = node_allocator::allocate();
    n->next = 0;
    __STL_TRY {
      construct(&n->val, obj);
      return n;
    }
    __STL_UNWIND(node_allocator::deallocate(n));
  }
  // 节点释放函数
  void delete_node(node* n){
    destory(&n->val);
    node_allocator::deallocate(n);
  }

  // 这个就是那个篮子
  vector<node*, Alloc> buckets;
  size_type num_elements;

  

  // 确定元素应处的篮子处
  // 四个版本
  // 1.接受value和bucket个数
  size_type bkt_num(const value_type& obj, size_t n) const {
    return bkt_num_key(get_key(obj), n);
  }
  // 2.接受value
  size_type bkt_num(const value_type& obj) const {
    return bkt_num_key(get_key(obj));
  }
  // 3.接受key
  size_type bkt_num_key(const key_type& key) const {
    return bkt_num_key(key, buckets.size());
  }
  // 4.接受key和bucket个数
  // 其他三个的最终归宿都是这里
  size_type bkt_num_key(const key_type& key, size_t n) const {
    return hash(key) % n;
  }


public:
  hashtable(size_type n, const HashFcn& hf, const EqualKey& eql) : hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0) {
    initialize_buckets(n);
  }

  // 插入元素，不重复
  pair<iterator, bool> insert_unique(const value_type& obj){
    resize(num_elements + 1); // 判断是否需要重新分配vector的空间
    return insert_unique_noresize(obj);
  }
  // 插入元素，允许重复
  iterator insert_equal(const value_type& obj){
    resize(num_elements + 1);
    return insert_equal_noresize(obj);
  }

private:
  void initialize_buckets(size_type n) {
    // 找到了合适的空间大小
    const size_type n_buckets = next_size(n);
    buckets.reserve(n_buckets);
    buckets.insert(buckets.end(), n_buckets, (node*)0);
    num_elements = 0;
  }

  // 返回接近n并大于n的质数
  size_type next_size(size_type n) const {
    return __stl_next_prime(n);
  }
public:
  // bucket 的个数，也就是buckets vector的大小
  size_type bucket_count() const { 
    return buckets.size();
  }
  
  iterator find(const key_type& key){
    size_type n = bkt_num_key(key);

    node* first;
    // 就是找到桶子然后从桶子里遍历去
    for(first = buckets[n];first &7 !equals(get_key(first->val), key); first = first->next){}
    // 要是没找到，返回的iterator就是个空的
    return iterator(first, this);
  }

  size_type count(const key_type& key) const {
    const size_type n = bkt_num_key(key);
    size_type result = 0;

    for(const node* cur = buckets[n];cur;cur=cur->next)
      if(equals(get_key(cur->val),key))
        ++result;
    return result;
  }

  size_type max_bucket_count() const {
    return __stl_prime_list[__stl_num_primes-1];
  }

private:
  // 以下是在外面实现的
  void resize(size_type num_elements_hint);
  pair<typename iterator, bool> insert_unique_noresize(const value_type& obj);
  typename iterator insert_equal_noresize(const value_type& obj)

public:
  void clear();
  void copy_from(const hashtable& ht);


private:
  // 28个质数，大概是二倍的关系
  static const int __stl_num_primes = 28;
  static const unsigned long __stl_prime_list[__stl_num_primes] = 
  {
    53, 97, 193, 389, 769,
    1543, 3079, 6151, 12289, 24593,
    49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 401653189, 805306457,
    1610612741, 3221225473ul, 4294967291ul
  };

  /**
   * @brief 找到28个质数中，最接近并大于n的那个质数
   * 
   * @param n 
   * @return unsigned 
   */
  inline unsigned long __stl_next_prime(unsigned long n) {
    const unsigned long* first = __stl_prime_list;
    const unsigned long* last = __stl_prime_list + __stl_num_primes;
    // lower_bound()是泛型算法
    const unsigned long* pos = lower_bound(first, last, n);
    return pos == last ? *(last - 1) : *pos;
  }

  

private:
  // 泛化
  template <class Key> struct hash { };

  inline size_t __stl_hash_string(const char* s){
    unsigned long h = 0;
    for(; *s;++s)
      h = 5*h + *s;
    return size_t(h);
  }

  // 特化
  __STL_TEMPLATE_NULL struct hash<char*> {
    size_t operator()(char* x) const { 
      return __stl_hash_string(s);
    }
  };
  __STL_TEMPLATE_NULL struct hash<const char*> {
    size_t operator()(const char* x) const { 
      return __stl_hash_string(s);
    }
  };
  __STL_TEMPLATE_NULL struct hash<char> {
    size_t operator()(char x) const { 
      return x;
    }
  };
  __STL_TEMPLATE_NULL struct hash<unsigned char> {
    size_t operator()(unsigned char x) const { 
      return x;
    }
  };
  __STL_TEMPLATE_NULL struct hash<signed char> {
    size_t operator()(signed char x) const { 
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
};


/**
 * @brief 表格是否重建
 * 表格是否重新的判断条件：元素个数和bucket vector的大小相比较
 * @tparam V 
 * @tparam K 
 * @tparam HF 
 * @tparam Ex 
 * @tparam Eq 
 * @tparam A 
 * @param num_elements_hint 
 */
template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::resize(size_type num_elements_hint) {
  // 保存桶子的总数
  const size_type old_n = buckets.size();
  if(num_elements_his > old_n) { // 判断是否真的需要重新配置
    const size_type n = next_size(num_elements_hint); // 找到下一个质数
    if(n > old_n) {
      vector<node*, A> tmp(n, (node*)0); // 设立新的buckets
      __STL_TRY {
        // 处理旧的bucket
        for(size_type bucket = 0; bucket < old_n; ++bucket) {
          node* first = buckets[bucket]; // 指向节点对应的起始节点
          while(first){
            // 找到它的新归宿
            size_type new_bucket = bkt_num(first->val, n);
            // 令旧的bucket指向下一个节点
            buckets[bucket] = first->next;
            // 把这个节点添加到新的bucket上去
            first->next = tmp[new_bucket];
            tmp[new_bucket] = first;
            // 准备为下一个节点寻找归宿
            first = buckets[bucket];
          }
        }
        // 新旧连个buckets互换，在内部直接完成了空间的扩展
        buckets.swap(tmp);
      }
    }
  }
}

/**
 * @brief 不允许重复插入
 * 
 * @tparam V 
 * @tparam K 
 * @tparam HF 
 * @tparam Ex 
 * @tparam Eq 
 * @tparam A 
 * @param obj 
 * @return pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool> 
 */
template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool> hashtable<V, K, HF, Ex, Eq, A>::insert_unique_noresize(const value_type& obj) {
  // 找到它应该放的位置
  const size_type n = bkt_num(obj);
  // 从这个桶子的头部开始运转
  node* first = buckets[n];

  for(node* cur = first; cur; cur = cur->next)
    if(equals(get_key(cur->val), get_key(obj))) // 如果链表中的某键值和它相同，就不插入，返回
      return pair<iterator, bool>(iterator(cur, this), false);

  // 检查完了链表，链表中没有和它相同的值  
  node* tmp = new_node(obj);
  // 就把它塞入链表的头部
  tmp->next = first;
  buckets[n] = tmp;
  // 节点数累加
  ++num_elements;
  return pair<iterator, bool>(iterator(tmp, this), true);
}


template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator hashtable<V, K, HF, Ex, Eq, A>::insert_equal_noresize(const value_type& obj) {
  // 确定插入的位置
  const size_type n = bkt_num(obj);
  node* first = buckets[n];

  for(node* cur = first; cur; cur=cur-next)
    if(equals(get_key(cur->val), get_key(obj))){ // 找到相同的那个位置，直接插入进去
      node* tmp = new_node(obj);
      tmp->next = cur->tmp;
      cur->next = tmp;
      ++num_elements;
      return iterator(tmp, this);
    }
  node* tmp = new_node(obj);
  tmp->next = first;
  buckets[n] = tmp;
  ++num_elements;
  return iterator(tmp, this);
}

template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<V, K, HF, Ex, Eq, A>::clear() {
  // 针对每个bucket
  for (size_type i = 0; i < buckets.size(); ++i) {
    node* cur = buckets[i];
    // 释放每个桶子里的每个节点
    while(cur!=0) {
      node* next = cur->next;
      delete_node(cur);
      cur = next;
    }
    buckets[i] = 0; // 桶子内节点释放干净后指null
  }
  num_elements = 0;
  // vector 的空间没有释放掉
}

template <class V, class K, class HF, class Ex, class Eq, class A>
void hashtable<v, K, HF, Ex, Eq, A>::copy_from(const hashtable& ht) {
  // 先把自己的清除掉(它还剩下vector的空间)
  buckets.clear();
  // 使自己的空间大小与要复制的空间大小一致(如果大于要复制的就不管)
  buckets.reserve(ht.buckets.size());
  // 给自己添加点起始指针
  buckets.insert(buckets.end(), ht.buckets.size(), (node*)0);
  __STL_TRY {
    for(size_type i=0; i < ht.buckets.size(); ++i) { // 针对vector中的每个桶子
      if(const node* cur = ht.buckets[i]){
        node* copy = new_node(cur->val);
        buckets[i] = copy;

        for(node* next = cur->next; next; cur = next, next = cur->next){ // 针对每一个桶子的节点
          copy->next = new_node(next->val);
          copy = copy->next;
        }        
      }
    }
    // 重新设定节点数量
    num_elements = ht.num_elements;
  }
  __STL_UNWIND(clear());
}