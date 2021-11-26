#include <list>
#include <unordered_map>
#include <assert.h>
#include <boost/functional/hash.hpp>

#define CACHESIZE 65535

using namespace std;

template <typename Container> // we can make this generic for any container [1]
struct container_hash {
  std::size_t operator()(Container const& c) const {
    return boost::hash_range(c.begin(), c.end());
  }
};

template <class KEY_T, class VAL_T, typename Container> class LRUCache{
private:
  void clean(void){
    while(item_map.size()>cache_size){
      auto last_it = item_list.end(); last_it --;
      item_map.erase(last_it->first);
      item_list.pop_back();
    }
  };
public:
  LRUCache(int cache_size_):cache_size(cache_size_){
    ;
  };

  void put(const KEY_T &key, const VAL_T &val){
    auto it = item_map.find(key);
    if(it != item_map.end()){
      item_list.erase(it->second);
      item_map.erase(it);
    }
    item_list.push_front(make_pair(key,val));
    item_map.insert(make_pair(key, item_list.begin()));
    clean();
  };
  bool exist(const KEY_T &key){
    return (item_map.count(key)>0);
  };
  VAL_T get(const KEY_T &key){
    assert(exist(key));
    auto it = item_map.find(key);
    item_list.splice(item_list.begin(), item_list, it->second);
    return it->second->second;
  };

private:
  list< pair<KEY_T,VAL_T> > item_list;
  unordered_map<KEY_T, decltype(item_list.begin()), container_hash<Container>> item_map;
  size_t cache_size;

};
