#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>

template <typename T>
class FreeList
{
public:
        struct Handle
        {
                size_t index;
                size_t generation;
        };
        class ActiveIterator
        {
                const FreeList<T> &pool;
                size_t i;

        public:
                ActiveIterator(const FreeList<T> &pool_, size_t start)
                    : pool(pool_), i(start)
                {
                        // skip to first active slot
                        while (i < pool.list.size() && !pool.occupied[i])
                                ++i;
                }

                bool operator!=(const ActiveIterator &other) const { return i != other.i; }

                Handle operator*() const { return {i, pool.generation[i]}; }

                ActiveIterator &operator++()
                {
                        do
                        {
                                ++i;
                        } while (i < pool.list.size() && !pool.occupied[i]);
                        return *this;
                }
        };

        ActiveIterator begin() const { return ActiveIterator(*this, 0); }
        ActiveIterator end() const { return ActiveIterator(*this, list.size()); }

        std::vector<T> list;
        std::vector<size_t> freelist;
        std::vector<size_t> generation;
        std::vector<bool> occupied;
        FreeList() = default;
        ~FreeList() = default;

        Handle getHandleByIndex(size_t index)
        {
                return {index, generation[index]};
        }

        Handle insert(const T &value)
        {
                if (!freelist.empty())
                {
                        size_t index = freelist.back();
                        freelist.pop_back();
                        list[index] = value;
                        occupied[index] = true;

                        return {index, generation[index]};
                }
                else
                {
                        list.push_back(value);
                        generation.push_back(0);
                        occupied.push_back(true);
                        return {list.size() - 1, 0};
                }
        }

        void remove(Handle handle)
        {
                if (!is_valid(handle))
                        throw std::runtime_error("Invalid handle");

                size_t index = handle.index;
                occupied[index] = false;
                generation[index]++;
                freelist.push_back(index);
        }

        T &get(Handle handle)
        {
                if (!is_valid(handle))
                        throw std::runtime_error("Invalid handle");

                return list[handle.index];
        }

        const T &get(Handle handle) const
        {
                if (!is_valid(handle))
                        throw std::runtime_error("Invalid handle");

                return list[handle.index];
        }
        bool isOccupied(size_t index) const
        {
                return index < occupied.size() && occupied[index];
        }

        bool is_valid(Handle handle) const
        {
                return handle.index < list.size() &&
                       occupied[handle.index] &&
                       generation[handle.index] == handle.generation;
        }
};