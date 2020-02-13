#include<vector>
#include<algorithm>
#include<exception>
#include<iterator>
#include<iostream>
#include<utility>
#include<list>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
private:
    static constexpr size_t START_SIZE = 13;
    static constexpr size_t K_FILL = 2;

    Hash hasher;
    size_t count_element = 0;

    typedef std::pair<KeyType, ValueType> element_type;

    std::vector<std::list<element_type>> data;

    template<typename element_type, typename table_iterator_type, typename list_iterator_type>
    class _iterator {
    private:
        table_iterator_type current_index;
        list_iterator_type current_itr;
        table_iterator_type realend;
    public:
        _iterator(const _iterator &other) : current_index(other.current_index),
                                            current_itr(other.current_itr),
                                            realend(other.realend) {}

        _iterator(const table_iterator_type &ind, const list_iterator_type &itr, const table_iterator_type &end) : current_index(ind), 
                                                                                                                   current_itr(itr),
                                                                                                                   realend(end) {}

        _iterator(const table_iterator_type &ind, const table_iterator_type &end) : current_index(ind), 
                                                                                    realend(end) {}

        _iterator() {}

        table_iterator_type getNext(table_iterator_type cur) {
            while (cur != realend && (*cur).empty()) cur++;
            return cur;
        }

        _iterator operator++() {
            if (current_index == realend) return (*this);
            ++current_itr;
            if (current_itr == (*current_index).end()) {
                current_index++;
                current_index = getNext(current_index);
                if (current_index != realend) {
                    current_itr = (*current_index).begin();
                }
            }
            return (*this);
        }

        _iterator operator++(int) {
            auto old_value = (*this);
            ++(*this);
            return old_value;
        }

        element_type& operator*() const {
            return *reinterpret_cast<element_type*>(&(*current_itr));
        }

        element_type* operator->() const {
            return reinterpret_cast<element_type*>(&(*current_itr));
        }

        bool operator==(const _iterator &other) const {
            if (current_index == realend && other.current_index == realend) return true;
            return static_cast<bool>(current_index == other.current_index && current_itr == other.current_itr);
        }

        bool operator!=(const _iterator &other) const {
            return !((*this) == other);
        }
    };

public:

    HashMap(Hash _hasher = Hash()) : hasher(_hasher) {
        data.resize(START_SIZE);
    }

    template<class Iter>
    HashMap(Iter begin, Iter end, Hash _hasher = Hash()) : hasher(_hasher) {
        data.resize(START_SIZE);
        while (begin != end) {
            insert(*(begin++));
        }
    } 

    HashMap(std::initializer_list<element_type> initializer_list, Hash _hasher = Hash()) :
        HashMap(initializer_list.begin(), initializer_list.end(), _hasher) {}

    size_t size() const {
        return count_element;
    }

    bool empty() const {
        return static_cast<bool>(count_element == 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    void rebuild(size_t new_size) {
        std::vector<std::list<element_type>> new_data(new_size);
        for (auto it = begin(); !(it == end()); ++it) {
            auto Key = (*it).first;
            auto Value = (*it).second;
            new_data[hasher(Key) % new_size].push_back({Key, Value});
        }
        new_data.swap(data);
    }

    void insert(const element_type &elem) {
        size_t index = hasher(elem.first) % data.size();
        bool contains = false;
        for (const auto &x : data[index]) {
            if (x.first == elem.first) contains = true;
        }
        if (contains) return;
        count_element++;
        data[index].push_back(elem);
        if (count_element * K_FILL > data.size()) rebuild(data.size() * K_FILL);
    }

    void erase(const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        bool contains = false;
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if ((*it).first == Key) {
                contains = true;
                data[index].erase(it);
                break;
            }
        }
        if (contains) count_element--;
    }

    typedef _iterator<std::pair<const KeyType, ValueType>, typename std::vector<std::list<element_type>>::iterator,
                                                           typename std::list<element_type>::iterator> iterator;

    typedef _iterator<const std::pair<const KeyType, ValueType>, typename std::vector<std::list<element_type>>::const_iterator,
                                                                 typename std::list<element_type>::const_iterator> const_iterator;

    iterator begin() {
        auto current_index = data.begin();
        for (; current_index != data.end(); current_index++) {
            if (!(*current_index).empty()) {
                break;
            }
        }
        if (current_index == data.end())
            return iterator(current_index, data.end());
        return iterator(current_index, (*current_index).begin(), data.end());
    }

    iterator end() {
        return iterator(data.end(), data.end());
    }

    const_iterator begin() const {
        auto current_index = data.begin();
        for (; current_index != data.end(); current_index++) {
            if (!(*current_index).empty()) {
                break;
            }
        }
        if (current_index == data.end())
            return const_iterator(current_index, data.end());
        return const_iterator(current_index, (*current_index).begin(), data.end());
    }

    const_iterator end() const {
        return const_iterator(data.end(), data.end());
    }

    iterator find(const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if ((*it).first == Key) {
                return iterator(data.begin() + index, it, data.end());
            }
        }
        return end();
    }

    const_iterator find(const KeyType &Key) const {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if ((*it).first == Key) {
                return const_iterator(data.begin() + index, it, data.end());
            }
        }
        return end();
    }

    ValueType& operator[](const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if ((*it).first == Key) {
                return (*it).second;
            }
        }
        insert({Key, ValueType()});
        return (*find(Key)).second;
    }

    const ValueType& at(const KeyType &Key) const {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if ((*it).first == Key) {
                return (*it).second;
            }
        }
        throw std::out_of_range("Out of range"); 
    }

    void clear() {
        data.clear();
        data.resize(START_SIZE);
        count_element = 0;
    }
};