/**
 * @brief vector的大部分操作都在此处
 * 
 * @version G2.9
 * @tparam T 
 * @tparam Alloc 
 */
template <class T, class Alloc = alloc>
class vector {
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type* iterator;
  typedef value_type& reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
protected:
  // simple_alloc是SGI STL的空间配置器
  typedef simple_alloc<value_type, Alloc> data_allocator;
  iterator start;
  iterator finish;
  // 整个空间的终点
  iterator end_of_storage;

  // position:插入位置
  void insert_aux(iterator position, const T& x);

  void deallocate() { // 将所有空间全部释放
    if (start)
      data_allocator::deallocate(start, end_of_storage - start);
  }

  // 这是用于初始化的函数
  // 填充并予以初始化
  void fill_initialized(size_type n, const T& value) {
    start = allocate_and_fill(n, value);
    finish = start + n;
    end_of_storage = finish;
  }

  // 配置空间并填满内容
  // 配置而后填充 
  iterator allocate_and_fill(size_type n, const T& x) {
    iterator result = data_allocator::allocate(n);
    // 它会根据第一参数的型别特性决定使用算法fill_n() 或反复调用 construct()来完成任务
    uninitialized_fill_n(result, n, x);
    return result;
  }

public:
  iterator begin() {
    return start;
  }

  iterator end() {
    return finish;
  }

  size_type size() const {
    return size_type(end() - begin());
  }

  size_type capacity() const {
    return size_type(end_of_storage - begin());
  }

  bool empty() const {
    return begin() == end();
  }

  vector() : start(0), finish(0), end_of_storage(0) { /* 默认初始化 */ }
  vector(size_type n, const T& value) { // size_t可以提高代码的可移植性、有效性和可读性
    fill_initialized(n, value);
  }
  vector(int n, const T& value) { // 使用n个相同的值初始化
    fill_initialized(n, value);
  }
  vector(long n, const T& value) {
    fill_initialized(n, value);
  }
  explicit vector(size_type n) { // 通过explict关键字来指定构造函数为显示，使它不能发生隐式类型转换
    fill_initialized(n, T());
  }

  ~vector() {
    destory(start, finish);
    deallocate();
  }

  reference operator[](size_type n) {
    return *(begin() + n);
  }

  reference front() {
    return *begin();
  }

  reference back() {
    return *(end() - 1);
  }

  void push_back(const T& x) {
    if(finish != end_of_storage) { // 在还有利用空间的时候
      construct(finish, x);
      ++finish; // 调整水位高度
    } else { // 在没有了可利用空间的时候
      insert_aux(end(), x);
    }
  }

  void pop_back() {
    // 直接向前提一个然后销毁后面的
    --finish;
    destory(finish);
  }

  iterator erase(iterator position) { // 清除某个位置上的元素
    if(position + 1 != end())
      copy(position + 1, finish, position); // 后续元素向前移动
    --finish;
    destory(finish);
    return position;
  }

  void resize(size_type new_size, const T& x) { // 重新规划空间
    if(new_size < size()) // 要是小于原空间就删掉多的
      erase(begin() +new_size, end());
    else // 要是大于原空间就在后面增加空间
      insert(end(), new_size, end(), x);
  }

  void resize(size_type new_size) { // 这两个区别就是数据初值不同
    resize(new_size, T());
  }

  void clear() { // 这就是清全部，好多函数就是使用一下之前定义好的函数。
    erase(begin(), end());
  }


};

/**
 * @brief 这个是关于内除处理的相关代码
 * 它会被push_back()、insert()等会使得内存扩充的函数调用
 * @tparam T 
 * @tparam Alloc 
 * @param position 
 * @param x 
 */
template <class T, class Alloc>
void vector<T,Alloc>::insert_aux(iterator position, const T& x) {
  if (finish != end_of_storage) { // 仍有剩余空间(因为它也会被insert调用，所以重新检查一次)
    construct(finish, *(finish - 1));
    ++finish;
    T x_copy = x;
    copy_backward(position, finish - 2, finish - 1);
    *position = x_copy;
  } else { // 已无剩余空间
    // 记录原来空间的大小
    const size_type old_size = size();
    // 二倍增长(0的二倍仍然是0，所以对0特殊处理)
    const size_type len = old_size != 0 ? 2*old_size : 1;
    // 前半段用来防止原数据，后半段用来放置新数据
    iterator new_start = data_allocator::allocate(len);
    iterator new_finish = new_start;
    try {
      // 将vector原来的内容拷贝到新的vector中
      new_finish = uninitialized_copy(start, position, new_start);
      construct(new_finish, x); // 为新元素设初值x
      ++new_finish; // 调整水位
      // 拷贝安插点后面的内容(因为它也会被insert调用)
      new_finish = uninitialized+copy(position, finish, new_finish);
    } catch(...) {
      // "commit or rollback" semantics.
      destory(new_start, new_finish);
      data_allocator::deallocate(new_start, len);
      throw;
    }
    // 解构并释放原vector
    destory(begin(), end());
    deallocate();
    // 调整迭代器，指向新的vector
    start = new_start;
    finish = new_finish;
    end_of_storage = new_start + len;

  }
}

/**
 * @brief insert函数的具体实现
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param position 
 * @param n 
 * @param x 
 */
template <class T, class Alloc>
void vector<T, Alloc>::insert(iterator position, size_type n, const T& x) {
  if(n != 0) {
    if(size_type(end_of_storage - finish) >= n) {
      // 备用空间大于等于新增元素个数
      T x_copy = x;
      // 计算插入点之后已经存在的元素的个数
      const size_type elems_after = finish - position;
      iterator old_finish = finish;
      if(elems_after > n){
        // 插入点之后的现有元素个数 大于 新增元素个数
        // 复制[finish-n, finish)的元素到从finish开始的空间
        uninitilized_copy(finish - n, finish, finish);
        finish += n; // 将vector尾端标记后移
        // 复制[position, old_finish)指定的序列到[,old_finish)中去，目的区间开始位置会自动计算
        copy_backward(position, old_finish - n, old_finish);
        fill(position, position + n, x_copy); // 从插入点开始填充新数据
      }
      else {
        // 插入点之后的现有元素个数 小于等于 新增元素个数
        // 这个就是先把大于的那部分赋给了从finish开始的位置，再把position到old_finish的值移到后面去，再给空出来的位置赋值 
        // 复制给定的值(x_copy)赋给从finish开始的长度为n-elems_after的内存区域
        uninitialized_fill_n(finish, n - elems_after, x_copy);
        finish += n - elems_after;
        uninitialized_copy(position, old_finish, finish);
        finish += elems_after;
        fill(position, old_finish, x_copy);
      }
    }
    else {
      // 备用空间小于 新增元素个数 (需要配置额外的内存)
      // 首先决定新长度：就长度的两倍，或者接长度+新增元素个数
      const size_type old_size = size();
      const size_type len = old_size + max(old_size, n);
      // 一下配置新的vector空间
      iterator new_start = data_allocator::allocate(len);

      iterator new_finish = new_start;
      __STL_TRY {
        // 以下首先将旧vector的插入点之前的元素复制到新空间
        new_finish = uninitialized_copy(start, position, new_start);
        // 以下再将新增元素填入新空间
        new_finish = uninitialized_fill_n(new_finish, n, x);
        // 以下再将插入点后面的原数据复制到新空间
        new_finish = uninitialized_copy(position, finish, new_finish);
      }
#ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        // 如果有异常发生，实现 "commit or rollback" semantics
        destory(new_start, new_finish);
        data_allocator::deallocate(new_start, len);
        throw;
      }
#endif /* __STL_USE_EXCEPTIONS */
      // 以下清除并释放旧的vector
      destory(start, finish);
      deallocate();
      // 以下调整水位标记
      start = new_start;
      finish = new_finish;
      end_of_storage = new_start + len;
    }
  }
}


/**
 * @brief 下面是G4.9版
 * 这个是在2.9版的基础上扩充
 * 它是将2.9的框架扩大了，但是内容并没有丰富多少，会显得皮包骨头
 * 应该是为了之后的进一步扩充做准备的。
 * 
 * @tparam _Tp 
 * @tparam _Alloc 
 */
template <typename _Tp, typename _Alloc>
struct _Vector_base {
  typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template rebind<_Tp>::other _Tp_alloc_type;
  typedef typename __gnu_cxx::__alloc_traits<_Tp_alloc_type>::pointer pointer;
};

template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
class vector : protected _Vector_base<_Tp, _Alloc>{
  ...
  typedef _Vector_base<_Tp, _Alloc> _Base;
  typedef typename _Base::pointer pointer;
  typedef __gnu_cxx::__normal_iterator<pointer, vector> iterator;
};

using std::iterator_traits;
using std::iterator;
template <typename _Iterator, typename _Container>
class __normal_iterator {
protected:
  _Iterator _M_current;
  typedef iterator_traits<_Iterator> __traits_type;
public:
  typedef _Iterator iterator_type;
  typedef typename __traits_type::iterator_category iterator_category;
  typedef typename __traits_type::value_type value_type;
  typedef typename __traits_type::difference_type difference_type;
  typedef typename __traits_type::reference reference;
  typedef typename __traits_type::pointer pointer;

  _GLIBCXX_CONSTEXPR __normal_iterator() _GLIBCSS_NOEXCEPT:_M_current(_iterator()) { }
};

template <typename _Tp>
class allocator : public __glibcxx_base_allocator<_Tp>{
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef _Tp* pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp& reference;
  typedef const _Tp& const_reference;
  typedef _Tp value_type;
  ...
  // Inherit everything else.
};