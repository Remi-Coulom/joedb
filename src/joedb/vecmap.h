#ifndef joedb_vecmap_declared
#define joedb_vecmap_declared

#include <vector>
#include <limits>
#include <cstdint>

namespace joedb
{
 template<typename Index, typename T>
 class vecmap
 {
  private:
   std::vector<T> v;
   std::vector<std::uint8_t> used;

  public:
   class const_iterator
   {
    private:
     const vecmap<Index, T> &map;
     std::size_t pos;

    public:
     const_iterator(const vecmap<Index, T> &map, std::size_t pos):
      map(map),
      pos(pos)
     {
     }

     bool operator==(const const_iterator &it) const
     {
      return &map == &it.map && pos == it.pos;
     }

     bool operator!=(const const_iterator &it) const
     {
      return !(*this == it);
     }

     Index key() const {return Index(pos);}
     const T &value() const {return map.v[pos];}
   };

   const_iterator begin() const
   {
   }

   const_iterator end() const
   {
    return const_iterator{*this, std::numeric_limits<std::size_t>::max()};
   }

   const_iterator find(Index i) const
   {
    if (std::size_t(i) < v.size() && bool(used[std::size_t(i)]))
     return const_iterator{*this, i};
    else
     return end();
   }

   T &operator[](Index i)
   {
    if (std::size_t(i) >= v.size())
    {
     v.resize(std::size_t(i) + 1);
     used.resize(std::size_t(i) + 1, false);
    }

    used[std::size_t(i)] = true;

    return v[std::size_t(i)];
   }
 };
}

#endif
