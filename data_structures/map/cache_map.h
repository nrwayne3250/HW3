#ifndef DOCUMENTS_CACHE_MAP_H
#define DOCUMENTS_CACHE_MAP_H

#include <functional>
#include "data_structures/map/maybe.h"
#include "pthread.h"
#include "map_impl.h"
#include "string"
namespace data_structures {
namespace map {

template<typename KeyType, typename ValueType>
class CacheMap {
public:
    typedef std::function<bool(const KeyType&, const KeyType&)> KeyComparerFn;
    typedef std::function<uint32_t(const KeyType&)> HashCalculator;
    typedef std::function<ValueType()> ValueFactory;

private:
    typedef std::pair<KeyType, ValueType> MapEntry;
    typedef std::vector<MapEntry> MapList;
    uint32_t size_;
    std::unique_ptr<MapList[]> storage_;
    const KeyComparerFn key_comparer_;
    const HashCalculator hash_calculator_;
    const uint32_t capacity_;
    const ValueType empty_value_;
    pthread_rwlock_t lockrw ;


public:
    CacheMap(const KeyComparerFn key_comparer,
             const HashCalculator hash_calculator,
             const uint32_t capacity, const ValueType empty_value)
            : key_comparer_(key_comparer), hash_calculator_(hash_calculator),
              capacity_(capacity), empty_value_(empty_value), storage_(new MapList[capacity]){

        pthread_rwlock_init(&lockrw,NULL);
    }

    ValueType Get(const KeyType& key, ValueFactory create_value) {

           pthread_rwlock_rdlock(&lockrw);
        Maybe<ValueType> value = Get(key);
        pthread_rwlock_unlock(&lockrw);
        if(value.IsPresent()){

            return value.Value();
        }

          // pthread_rwlock_unlock(&lockrw);

            std::string blah = create_value();
            int ret = pthread_rwlock_wrlock(&lockrw);
            if(!value.IsPresent()){
            Put(key, blah);}


        pthread_rwlock_unlock(&lockrw);

        return blah;

    }
    void Put(const KeyType& key, const ValueType& value) {
        int index = GetIndex(key);
        MapList& list = storage_.get()[index];
        MapEntry new_entry = {key,value};
        for(auto it = list.begin();
            it != list.end(); ++it) {
            MapEntry& entry = *it;
            if(key_comparer_(entry.first, key)) {
                list.erase(it);
                list.emplace_back(new_entry);
                return;
            }
        }
        list.emplace_back(new_entry);
        ++size_;
    }
    Maybe<ValueType> Get(const KeyType& key) const {
        int index = GetIndex(key);
        MapList& list = storage_.get()[index];
        for(auto it = list.begin();
            it != list.end(); ++it) {
            MapEntry& entry = *it;
            if(key_comparer_(entry.first, key)) {
                return Maybe<ValueType>(entry.second);
            }
        }
        return EmptyMaybe(empty_value_);
    }

    int size() const {
        return (int)size_;
    }

private:

    uint32_t GetIndex(const KeyType& key) const {
        uint32_t hash = hash_calculator_(key);
        uint32_t index = hash % capacity_;
        return index;
    }


};

}  // namespace map
}  // namespace data_structures

#endif //DOCUMENTS_CACHE_MAP_H
