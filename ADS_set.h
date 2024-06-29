#ifndef ADS_SET_H
#define ADS_SET_H
#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator;
  //using key_compare = std::less<key_type>;                         // B+-Tree
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

private: 
  struct Element
  {
    key_type key;
    Element* ptr {nullptr};
  };

  Element** table {nullptr};
  size_type table_size = N;
  size_type current_size {0};
  float max_lf {0.7};

  void add(const key_type &key);
  void reserve(size_type n);
  void rehash(size_type n);
  Element* locate(const key_type &key) const;
  size_type h(const key_type &key) const 
  {
    return hasher{}(key) % table_size;
  }


public:
  ADS_set(): table{new Element*[N]{}}, table_size{N}
  {
  }
                                                          
  ADS_set(std::initializer_list<key_type> ilist): ADS_set() 
  { 
    insert(ilist); 
  }
  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set()
  { 
    insert(first,last); 
  }
  //PH2 KONSTRUKTOR
  ADS_set(const ADS_set &other) : ADS_set()
  {
    insert(other.begin(), other.end());
    return;
  }


  ~ADS_set()
  {

    for (size_type idx = 0; idx < table_size; ++idx)
    {
        Element* current = table[idx];
        while (current != nullptr)
        {
            Element* next = current->ptr;
            delete current;
            current = next;
        }
    }
    delete[] table;
  }

  //METHODEN
  size_type size() const 
  { 
    return current_size; 
  }
  bool empty() const 
  { 
    return current_size == 0; 
  }

  void insert(std::initializer_list<key_type> ilist) 
  { 
    insert(ilist.begin(), ilist.end()); 
  }

  size_type count(const key_type &key) const 
  { 
    return locate(key) != nullptr; 
  }
  
  template<typename InputIt> void insert(InputIt first, InputIt last);
  
  void dump(std::ostream &o = std::cerr) const;


  //PHASE 2
  void swap(ADS_set &other) noexcept
  {
    using std::swap;
    swap(table, other.table);
    swap(table_size, other.table_size);
    swap(current_size, other.current_size);
  }

  ADS_set &operator=(const ADS_set &other)
  {
    ADS_set temp(other);
    swap(temp);
    return *this;
  }


  ADS_set &operator=(std::initializer_list<key_type> ilist)
  {
    ADS_set temp(ilist);
    std::swap(table_size, temp.table_size);
    std::swap(table, temp.table);
    std::swap(current_size, temp.current_size);
    return *this;

  }

  std::pair<typename ADS_set<Key,N>::iterator,bool> insert(const key_type &key) 
  {
    if (auto e {locate(key)}) 
    {
      return {iterator{locate(key), table, table_size, h(key)},false};
    }
    reserve(current_size+1);
    add(key);
    return {iterator{locate(key), table, table_size, h(key)},true};
  }

  void clear()
  {
    ADS_set temp;
    swap(temp);
  }

  size_type erase(const key_type &key)
{
    size_type idx = h(key);
    Element* iter = table[idx];
    Element *prev = nullptr;
    while (iter)
    {
      if (key_equal{}(iter->key, key))
      {
        if (prev)
        {
          prev->ptr = iter->ptr;
        }
        else
          table[idx] = iter->ptr;
        delete iter;
        --current_size;
        return 1;
      }
      prev = iter;
      iter = iter->ptr;
    }
    return 0;
}

  iterator find(const key_type &key) const
  {
    return Iterator(locate(key), table, table_size, h(key));
  }


  const_iterator begin() const
  {
    for (size_type idx = 0; idx < table_size; ++idx)
    {
        if (table[idx] != nullptr)
        {
            return Iterator(table[idx], table, table_size, idx);
        }
    }
    return end();
  }

  const_iterator end() const
  {
    return Iterator(nullptr, table, table_size, table_size-1);
  }

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if (lhs.current_size != rhs.current_size) return false;
    for (const auto &k: lhs) if (!rhs.count(k)) return false;
    return true;
  }

  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs)
  {
    return !(lhs == rhs);
  }
  //ENDE PHASE 2
};


//ITERATOR
template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator 
{
  Element *e = nullptr;
  Element **table = nullptr;
  size_type sz = 0;
  size_type idx = 0;
  


public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit Iterator(Element *e = nullptr, Element** table = nullptr, size_type sz = 0, size_type idx = 0): e{e}, table{table}, sz{sz}, idx{idx}
  {}

  reference operator*() const 
  { 
    return e->key; 
  }
  pointer operator->() const 
  { 
    return &e->key; 
  }
  Iterator &operator++()
  {
    e = e->ptr;
    if (e != nullptr)
      return *this;
    while ((idx+1) < sz)
    {
      e = table[++idx];
      if (e != nullptr)
        break;
    }
    return *this;
  }


  Iterator operator++(int) 
  { 
    auto rc {*this}; ++*this; return rc; 
  }
  friend bool operator==(const Iterator &lhs, const Iterator &rhs) { return lhs.e == rhs.e; }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return !(lhs == rhs); }
};

//METHODEN

template <typename Key, size_t N>
typename ADS_set<Key, N>::Element* ADS_set<Key, N>::locate(const key_type &key) const
{
    size_type idx = h(key);
    if (table[idx] == nullptr) return nullptr;
    Element* temp = table[idx];
    while (temp != nullptr)
    {
        if (key_equal{}(temp->key, key))
        {
            return temp;
        }
        temp = temp->ptr;
    }

    return nullptr; 
}

template <typename Key, size_t N>
template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last)
{
    for (auto it {first}; it != last; ++it) {
      if (!count(*it)) 
        {
            reserve(size()+1);
            add(*it);
        }
      } 
}

template <typename Key, size_t N>
void ADS_set<Key,N>::reserve(size_type n) {
  if (n > max_lf * table_size)
    rehash(n);
}

template <typename Key, size_t N>
void ADS_set<Key, N>::add(const key_type &key) 
{
  if (locate(key) != nullptr) return;
  size_type idx {h(key)};
  Element* temp = new Element{key};
  temp->ptr = table[idx];
  table[idx] = temp;
  ++current_size;
}




template <typename Key, size_t N>
void ADS_set<Key, N>::rehash(size_type n) 
{
  size_type new_table_size = std::max(n, table_size*2);
  Element** new_table = new Element *[new_table_size] {};

  for (size_type idx = 0; idx < table_size; idx++) 
  {
    Element* temp = table[idx];
    while (temp != nullptr) 
    {
      Element* next = temp->ptr;
      size_type new_idx = hasher{}(temp->key) % new_table_size;
      temp->ptr = new_table[new_idx];
      new_table[new_idx] = temp;
      temp = next;
    }
  }
  delete[] table;
  table = new_table;
  table_size = new_table_size;
}


template <typename Key, size_t N>
void ADS_set<Key,N>::dump(std::ostream &o) const {
  o << "table_size = " << table_size << ", current_size = " << current_size << "\n";
  for (size_type idx {0}; idx < table_size; ++idx) {
    o << idx  << ": ";
    Element* neu = table[idx];
    while (neu != nullptr) // während neu kein nullpointer is. somit nicht auf ein nicht existierendes key zugegriffen wird
    {
      o << neu->key <<", ";
      neu = neu->ptr;
    }
    o << "\n";
  }
}
template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }
#endif // ADS_SET_H