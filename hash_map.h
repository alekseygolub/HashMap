#include<vector>
#include<algorithm>
#include<exception>
#include<iterator>
#include<iostream>
#include<utility>
#include<list>
#include<stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
private:
    static constexpr size_t START_SIZE = 13;
    static constexpr size_t K_FILL = 2;

    Hash hasher;
    size_t element_count = 0;

    typedef std::pair<KeyType, ValueType> Element;

    std::vector<std::list<Element>> data;

    template<typename Element, typename table_iterator_type, typename list_iterator_type>
    class _iterator {
    private:
        table_iterator_type table_itr;
        list_iterator_type list_itr;
        table_iterator_type real_end;
    public:
        _iterator(
            const _iterator &other)
            : table_itr(other.table_itr)
            , list_itr(other.list_itr)
            , real_end(other.real_end)
        { }

        _iterator(
            const table_iterator_type &ind,
            const list_iterator_type &itr,
            const table_iterator_type &end)
            : table_itr(ind)
            , list_itr(itr)
            , real_end(end)
        { }

        _iterator(
            const table_iterator_type &ind,
            const table_iterator_type &end)
            : table_itr(ind)
            , real_end(end)
        { }

        _iterator() {}

        table_iterator_type getNext(table_iterator_type it) {
            while (it != real_end && it->empty()) it++;
            return it;
        }

        _iterator operator++() {
            if (table_itr == real_end)
                return *this;
            ++list_itr;
            if (list_itr == table_itr->end()) {
                table_itr++;
                table_itr = getNext(table_itr);
                if (table_itr != real_end)
                    list_itr = table_itr->begin();
            }
            return *this;
        }

        _iterator operator++(int) {
            auto old_value = *this;
            ++(*this);
            return old_value;
        }

        Element& operator*() const {
            return *reinterpret_cast<Element*>(&(*list_itr));
        }

        Element* operator->() const {
            return reinterpret_cast<Element*>(&(*list_itr));
        }

        bool operator==(const _iterator &other) const {
            if (table_itr == real_end && other.table_itr == real_end)
                return true;
            return (table_itr == other.table_itr && list_itr == other.list_itr);
        }

        bool operator!=(const _iterator &other) const {
            return !(*this == other);
        }
    };

public:

    HashMap(Hash hasher = Hash()) : hasher(hasher) {
        data.resize(START_SIZE);
    }

    template<class Iter>
    HashMap(Iter begin, Iter end, Hash hasher = Hash()) : hasher(hasher) {
        data.resize(START_SIZE);
        while (begin != end) {
            insert(*(begin++));
        }
    } 

    HashMap(std::initializer_list<Element> initializer_list, Hash hasher = Hash()) :
        HashMap(initializer_list.begin(), initializer_list.end(), hasher) {}

    size_t size() const {
        return element_count;
    }

    bool empty() const {
        return (element_count == 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    void rebuild(size_t new_size) {
        std::vector<std::list<Element>> new_data(new_size);
        for (auto it = begin(); !(it == end()); ++it) {
            auto Key = it->first;
            auto Value = it->second;
            new_data[hasher(Key) % new_size].push_back({Key, Value});
        }
        new_data.swap(data);
    }

    void insert(const Element &elem) {
        size_t index = hasher(elem.first) % data.size();
        bool contains = false;
        for (const auto &x : data[index]) {
            if (x.first == elem.first)
                contains = true;
        }
        if (contains)
            return;
        element_count++;
        data[index].push_back(elem);
        if (element_count * K_FILL > data.size())
            rebuild(data.size() * K_FILL);
    }

    void erase(const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        bool contains = false;
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if (it->first == Key) {
                contains = true;
                data[index].erase(it);
                break;
            }
        }
        if (contains)
            element_count--;
    }

    using iterator = _iterator<
        std::pair<const KeyType, ValueType>,
        typename std::vector<std::list<Element>>::iterator,
        typename std::list<Element>::iterator>;

    using const_iterator = _iterator<
        const std::pair<const KeyType, ValueType>,
        typename std::vector<std::list<Element>>::const_iterator,
        typename std::list<Element>::const_iterator>;
    
    iterator begin() {
        auto table_itr = data.begin();
        for (; table_itr != data.end(); table_itr++) {
            if (!(table_itr->empty()))
                break;
        }
        if (table_itr == data.end())
            return iterator(table_itr, data.end());
        return iterator(table_itr, table_itr->begin(), data.end());
    }

    iterator end() {
        return iterator(data.end(), data.end());
    }

    const_iterator begin() const {
        auto table_itr = data.begin();
        for (; table_itr != data.end(); table_itr++) {
            if (!(table_itr->empty()))
                break;
        }
        if (table_itr == data.end())
            return const_iterator(table_itr, data.end());
        return const_iterator(table_itr, table_itr->begin(), data.end());
    }

    const_iterator end() const {
        return const_iterator(data.end(), data.end());
    }

    iterator find(const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if (it->first == Key)
                return iterator(data.begin() + index, it, data.end());
        }
        return end();
    }

    const_iterator find(const KeyType &Key) const {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if (it->first == Key)
                return const_iterator(data.begin() + index, it, data.end());
        }
        return end();
    }

    ValueType& operator[](const KeyType &Key) {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if (it->first == Key)
                return it->second;
        }
        insert({Key, ValueType()});
        return find(Key)->second;
    }

    const ValueType& at(const KeyType &Key) const {
        size_t index = hasher(Key) % data.size();
        for (auto it = data[index].begin(); it != data[index].end(); ++it) {
            if (it->first == Key)
                return it->second;
        }
        throw std::out_of_range("Out of range"); 
    }

    void clear() {
        data.clear();
        data.resize(START_SIZE);
        element_count = 0;
    }
};
