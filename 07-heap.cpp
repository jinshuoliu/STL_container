/**
 * @brief 下面三个是push_back的操作
 * 
 * @tparam RandomAccessIterator 
 * @param first 
 * @param last 
 */
template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
  // 此函数被调用的时候，新元素已经置于容器的最尾端
  __push_heap_aux(first, last, distance_type(first), value_type(first));
}

template <class RandomAccessIterator, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last, Distance*, T*) {
  // 这个是因为implicit repressentation heap的结构特性：新值必须位于容器底部
  __push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1))); // 根节点放在0了？
}

template <class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value) {
  Distance parent = (holeIndex - 1) / 2; // 找出父节点(这里为什么要先-1呢？它要是右子节点在除的时候就会约掉0.5，左子节点直接除尽(哦，这里根节点放在0了))
  while(holeIndex > topIndex && *(first + parent) < value) { first+parent是确定父节点的位置
    // 没到根节点呢，而且父节点小于新增的值
    *(first + holeIndex) = *(first + parent); // 上调位置
    holeIndex = parent;
    parent = (holeIndex - 1) / 2; // 新的父节点
  }
  *(first + holeIndex) = value; // 完成
}

/**
 * @brief 以下四个是pop_heap
 * 
 * @tparam RandomAccessIterator 
 * @param first 
 * @param last 
 */
template <class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
  __pop_heap_aux(first, last, value_type(first));
}

template <class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*) {
  // pop要把根节点的元素删除，所以把根节点的元素调整到尾节点
  __pop_heap(first, last-1, last-1, T(*(last-1)), distance_type(first));
}

template <class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T value, Distance*) {
  *result = *first; // 将根节点的元素填入到尾节点
  // 把根节点位置填入原尾节点的元素
  __adjust_heap(first, Distance(0), Distance(last - first), value);
}

template <class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value) {
  Distance topIndex = holeIndex; // 标记根节点以及它的右子节点的位置
  Distance secondChild = 2 * holeIndex + 2;
  while(secondChild < len) { // 限制范围
    // 比较左右节点哪个大一些
    if (*(first + secondChild) < *(first + (secondChild - 1)))
      secondChild--;
    // 将当前节点位置调整到较大的那个节点的位置
    *(first + holeIndex) = *(first + secondChild);
    holeIndex = secondChild;
    second Child= 2 * (second Child+ 1);
  }
  if(secondChild == len){ // 没有右节点了，只有左节点
    *(first + holeIndex) = *(first + (secondChild - 1));
    holeIndex = secondChild - 1;
  }
  // 此时不一定满足，需要执行一次上溯
  __push_heap(first, holeIndex, topIndex, value);
}

/**
 * @brief 排序操作
 * 执行完毕这个heap就失效了
 * @tparam RandomAccessIterator 
 * @param first 
 * @param last 
 */
template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
  while(last - first > 1)
    pop_heap(first, last--);
}


/**
 * @brief 下面两个是制作heap
 * 
 * @tparam RandomAccessIterator 
 * @param first 
 * @param last 
 */
template <class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
  __make_heap(first, last, value_type(first), distance_type(first));
}

template <class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*, Distance*) {
  // 长度为0、1不需要排序的
  if(last - first < 2) return;
  // 计算长度标记父节点
  Distance len = last - first;
  Distance parent = (len - 2) / 2;

  while(true) {
    // 重新为parent开始的子树排序
    __adjust_heap(first, parent, len, T(*(first + parent)));
    if(parent == 0) return; // 排到根节点就结束
    parent--; // 从后向前
  }
}