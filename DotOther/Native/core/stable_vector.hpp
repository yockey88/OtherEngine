/**
 * \file core/stable_vector.hpp
 **/
#ifndef DOTOTHER_STABLE_VECTOR_HPP
#define DOTOTHER_STABLE_VECTOR_HPP

#include <cstdint>
#include <array>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <list>

#include "core/dotother_defines.hpp"

namespace dotother {

  template <typename T, size_t N = 256>
	class StableVector {
    public:
      StableVector() = default;

      StableVector(const StableVector& other) {
        other.ForEach([this](const T& elem) mutable {
          EmplaceBackNoLock().second = elem;
        });
      }

      ~StableVector() {
        for (size_t i = 0; i < page_count; i++) {
          delete[] page_table[i];
        }
      }

      StableVector& operator=(const StableVector& other) {
        Clear();

        other.ForEach([this](const T& elem) mutable {
          EmplaceBackNoLock().second = elem;
        });

        return *this;
      }

      void Clear() {
        for (size_t i = 0; i < page_count; i++) {
          delete[] page_table[i];
        }

        elt_count = 0;
        capacity = 0;
        page_count = 0;
      }

      T& operator[](size_t index) {
        size_t page_idx = index / N;
        size_t elt_index = index - (page_idx * N);
        return page_table[page_idx]->elements[elt_index];
      }

      const T& operator[](size_t index) const {
        size_t page_idx = index / N;
        size_t elt_index = index - (page_idx * N);
        return page_table[page_idx]->elements[elt_index];
      }

      size_t GetElementCount() const { 
        return elt_count; 
      }

      std::pair<uint32_t, T&> Insert(T&& new_elt) {
        size_t page_idx = elt_count / N;

        if (elt_count >= capacity) {

          std::scoped_lock lock(mutex);
          if (elt_count >= capacity) {

            Page* new_page = new Page();
            if (page_idx >= page_count) {
              auto old_pages = page_count;

              page_count = (std::max)(uint64_t(16), page_count * 2);
              owner<Page*[]> new_page_table = new_owner<Page*[]>(page_count);
              std::memcpy(new_page_table.get(), page_table.load(), old_pages * sizeof(void*));

              page_table.exchange(new_page_table.get());
              page_tables.push_back(std::move(new_page_table));
            }

            page_table[page_idx] = new_page;

            capacity += N;
          }
        }

        uint32_t index = (++elt_count - 1);
        uint32_t elt_index = index - (page_idx * N);
        page_table[page_idx]->elements[elt_index] = std::move(new_elt);
        return { 
          index, 
          page_table[page_idx]->elements[elt_index]
        };
      }

      std::pair<uint32_t, T&> InsertNoLock(T&& new_elt) {
        size_t page_idx = elt_count / N;

        if (elt_count >= capacity) {
          std::scoped_lock lock(mutex);

          if (elt_count >= capacity) {

            Page* new_page = new Page();
            if (page_idx >= page_count) {
              uint64_t old_pages = page_count;
              page_count = (std::max)(uint64_t(16), page_count * 2);

              owner<Page*[]> new_page_table = new_owner<Page*[]>(page_count);
              std::memcpy(new_page_table.get(), page_table.load(), old_pages * sizeof(void*));

              page_table.exchange(new_page_table.get());
              page_tables.push_back(std::move(new_page_table));
            }

            page_table[page_idx] = new_page;

            capacity += N;
          }
        }

        uint32_t index = (++elt_count - 1);
        uint32_t elt_index = index - (page_idx * N);
        page_table[page_idx]->elements[elt_index] = std::move(new_elt);
        return { 
          index, 
          page_table[page_idx]->elements[elt_index] 
        };
      }

      std::pair<uint32_t, T&> EmplaceBack() {
        size_t page_idx = elt_count / N;

        if (elt_count >= capacity) {

          Page* new_page = new Page();
          if (page_idx >= page_count) {
            uint64_t old_pages = page_count;
            page_count = (std::max)(uint64_t(16), page_count * 2);

            owner<Page*> new_page_table = new_owner<Page*[]>(page_count);
            std::memcpy(new_page_table.get(), page_table.load(), old_pages * sizeof(void*));
            
            page_table.exchange(new_page_table.get());
            page_tables.push_back(std::move(new_page_table));
          }

          page_table[page_idx] = new_page;

          capacity += N;
        }

        uint32_t index = (++elt_count - 1);
        uint32_t elt_index = index - (page_idx * N);
        return { 
          index, 
          page_table[page_idx]->elements[elt_index] 
        };
      }

      std::pair<uint32_t, T&> EmplaceBackNoLock() {
        size_t page_idx = elt_count / N;

        if (elt_count >= capacity) {
          auto* new_page = new Page();

          if (page_idx >= page_count) {
            auto old_pages = page_count;
            page_count = (std::max)(uint64_t(16), page_count * 2);
            auto new_page_table = std::make_unique<Page*[]>(page_count);
            std::memcpy(new_page_table.get(), page_table.load(), old_pages * sizeof(void*));
            page_table.exchange(new_page_table.get());
            page_tables.push_back(std::move(new_page_table));
          }

          page_table[page_idx] = new_page;

          capacity += N;
        }

        uint32_t index = (++elt_count - 1);
        uint32_t elt_index = index - (page_idx * N);
        return { 
          index, 
          page_table[page_idx]->elements[elt_index] 
        };
      }

      template <typename Fn>
      void ForEach(Fn&& fn) {
        for (uint32_t i = 0; i < elt_count; ++i) {
          fn((*this)[i]);
        }
      }

      template <typename Fn>
      void ForEach(Fn&& fn) const {
        for (uint32_t i = 0; i < elt_count; ++i) {
          fn((*this)[i]);
        }
      }

    private:
      struct Page {
        std::array<T, N> elements;
      };

      std::shared_mutex mutex;
      std::list<owner<Page*[]>> page_tables;

      std::atomic<Page**> page_table;
      std::atomic<uint32_t> elt_count = 0;
      std::atomic<uint32_t> capacity = 0;
      uint64_t page_count = 0;
	};

} // namespace dotother

#endif // !DOTOTHER_STABLE_VECTOR_HPP