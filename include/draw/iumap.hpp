//===- include/draw/iumap.hpp --=--------------------------*- mode: C++ -*-===//
//*  _                              *
//* (_)_   _ _ __ ___   __ _ _ __   *
//* | | | | | '_ ` _ \ / _` | '_ \  *
//* | | |_| | | | | | | (_| | |_) | *
//* |_|\__,_|_| |_| |_|\__,_| .__/  *
//*                         |_|     *
//===----------------------------------------------------------------------===//
// Copyright © 2025 Paul Bowen-Huggett
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// “Software”), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// SPDX-License-Identifier: MIT
//===----------------------------------------------------------------------===//

/// \file iumap.hpp
/// \brief Provides iumap, an in-place unordered hash table.

#ifndef DRAW_IUMAP_HPP
#define DRAW_IUMAP_HPP

// Standard library
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

// Standard library: for quick and dirty trace output
#ifdef IUMAP_TRACE
#include <ostream>
#endif  // IUMAP_TRACE

namespace draw {

/// Returns true if the argument is a power of two and false otherwise.
///
/// \param n An integer value to check whether it is a power of two.
/// \returns True if the input value is a power of 2.
[[nodiscard]] constexpr auto is_power_of_two(std::unsigned_integral auto const n) noexcept -> bool {
#if defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 202002L
  return std::has_single_bit(n);
#else
  // If a number n is a power of 2 then bitwise & of n and n-1 will be zero.
  return n > 0U && !(n & (n - 1U));
#endif
}

/// An in-place unordered hash table.
///
/// \tparam Key  The key type
/// \tparam Mapped  The value type
/// \tparam Size  The number of key/value pairs that can be stored in the container
/// \tparam Hash  The type used to hash keys
/// \tparam KeyEqual  The type used to compare keys
template <typename Key, typename Mapped, std::size_t Size, typename Hash = std::hash<std::remove_cv_t<Key>>,
          typename KeyEqual = std::equal_to<Key>>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
class iumap {
  friend class iterator;
  struct member;

public:
  /// The key type
  using key_type = Key;
  using mapped_type = Mapped;
  using value_type = std::pair<Key const, Mapped>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using hasher = Hash;
  using key_equal = KeyEqual;

  using reference = value_type&;
  using const_reference = value_type const&;
  using pointer = value_type*;
  using const_pointer = value_type const*;

  template <typename T> class sentinel {};

  template <typename T>
    requires(std::is_same_v<T, member> || std::is_same_v<T, member const>)
  class iterator_type {
    friend iterator_type<std::remove_const_t<T>>;
    friend iterator_type<T const>;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = iumap::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<std::is_const_v<T>, value_type const*, value_type*>;
    using reference = std::conditional_t<std::is_const_v<T>, value_type const&, value_type&>;

    using container_type =
        std::conditional_t<std::is_const_v<T>, std::array<member, Size> const, std::array<member, Size>>;

    constexpr iterator_type() noexcept = default;
    constexpr iterator_type(T* const slot, container_type* const container) noexcept
        : slot_{slot}, container_{container} {
      this->move_forward_to_occupied();
    }

    constexpr bool operator==(iterator_type<std::remove_const_t<T>> const& other) const noexcept {
      return std::make_pair(slot_, container_) == std::make_pair(other.slot_, other.container_);
    }
    constexpr bool operator==(iterator_type<T const> const& other) const noexcept {
      return std::make_pair(slot_, container_) == std::make_pair(other.slot_, other.container_);
    }
    constexpr bool operator<=>(iterator_type<T const> const& other) const noexcept {
      return std::make_pair(slot_, container_) <=> std::make_pair(other.slot_, other.container_);
    }
    constexpr auto operator==(sentinel<T>) const noexcept { return slot_ == this->end_limit(); }
    constexpr auto operator!=(sentinel<T> other) const noexcept { return !operator==(other); }

    constexpr reference operator*() const noexcept { return *slot_->pointer(); }
    constexpr pointer operator->() const noexcept { return slot_->pointer(); }

    constexpr iterator_type& operator--() noexcept {
      --slot_;
      this->move_backward_to_occupied();
      return *this;
    }
    constexpr iterator_type& operator++() noexcept {
      ++slot_;
      this->move_forward_to_occupied();
      return *this;
    }
    constexpr iterator_type operator++(int) noexcept {
      auto const tmp = *this;
      ++*this;
      return tmp;
    }
    constexpr iterator_type operator--(int) noexcept {
      auto const tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator_type& operator+=(difference_type n) noexcept { return this->move_n(n); }
    constexpr iterator_type& operator-=(difference_type n) noexcept { return this->move_n(-n); }

    friend iterator_type operator+(iterator_type it, difference_type n) noexcept {
      auto result = it;
      result += n;
      return result;
    }
    friend iterator_type operator+(difference_type n, iterator_type it) noexcept { return it + n; }
    friend iterator_type operator-(iterator_type it, difference_type n) noexcept {
      auto result = it;
      result -= n;
      return result;
    }
    friend iterator_type operator-(difference_type n, iterator_type it) noexcept { return it - n; }

    T* raw() noexcept { return slot_; }

  private:
    constexpr auto end_limit() noexcept { return container_->data() + container_->size(); }
    constexpr void move_backward_to_occupied() noexcept {
      auto const* const limit = container_->data();
      while (slot_ >= limit && slot_->state != state::occupied) {
        --slot_;
      }
    }
    constexpr void move_forward_to_occupied() noexcept {
      auto const* const end = this->end_limit();
      while (slot_ < end && slot_->state != state::occupied) {
        ++slot_;
      }
    }
    constexpr iterator_type& move_n(difference_type n) noexcept {
      if (n == 0) {
        // nothing to do.
      } else if (n > 0) {
        for (; n > 0; --n) {
          ++(*this);
        }
      } else {
        for (n = -n; n > 0; --n) {
          --(*this);
        }
      }
      return *this;
    }

    T* slot_ = nullptr;
    container_type* container_ = nullptr;
  };

  using iterator = iterator_type<member>;
  using const_iterator = iterator_type<member const>;

  constexpr iumap() : hash_{hasher{}}, equal_{key_equal{}} {}
  constexpr explicit iumap(hasher const& hash, key_equal const& equal = key_equal{}) : hash_{hash}, equal_{equal} {}
  constexpr iumap(std::initializer_list<value_type> init, hasher const& hash = Hash{},
                  key_equal const& equal = key_equal{})
      : iumap{hash, equal} {
    assert(init.size() <= Size && "Initializer list is too long");
    for (auto const& v : init) {
      this->insert(v);
      assert(this->find(v.first)->first == v.first);
    }
  }
  iumap(iumap const& other) = default;
  iumap(iumap&& other) noexcept = default;
  ~iumap() noexcept = default;

  iumap& operator=(iumap const& other) = default;
  iumap& operator=(iumap&& other) noexcept = default;

  // Iterators
  [[nodiscard]] constexpr auto begin() noexcept { return iterator{v_.data(), &v_}; }
  [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{v_.data(), &v_}; }
  [[nodiscard]] constexpr auto cbegin() noexcept { return const_iterator{v_.data(), &v_}; }

  [[nodiscard]] constexpr auto end() noexcept { return iterator{v_.data() + v_.size(), &v_}; }
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{v_.data() + v_.size(), &v_}; }
  [[nodiscard]] constexpr auto cend() noexcept { return const_iterator{v_.data() + v_.size(), &v_}; }

  // Capacity
  [[nodiscard]] constexpr auto empty() const noexcept { return size_ == 0; }
  [[nodiscard]] constexpr auto size() const noexcept { return size_; }
  [[nodiscard]] constexpr auto max_size() const noexcept { return v_.max_size(); }
  [[nodiscard]] static constexpr auto capacity() noexcept { return Size; }

  // Modifiers
  void clear() noexcept;
  /// inserts elements
  std::pair<iterator, bool> insert(value_type const& value);
  /// inserts an element or assigns to the current element if the key already exists
  template <typename M> std::pair<iterator, bool> insert_or_assign(Key const& key, M&& value);
  /// inserts in-place if the key does not exist, does nothing if the key exists
  template <typename... Args> std::pair<iterator, bool> try_emplace(Key const& key, Args&&... args);
  /// erases elements
  iterator erase(iterator pos);

  // Lookup
  [[nodiscard]] constexpr iterator find(Key const& k);
  [[nodiscard]] constexpr const_iterator find(Key const& k) const;

  // Observers
  [[nodiscard]] constexpr hasher hash_function() const { return hash_; }
  [[nodiscard]] constexpr key_equal key_eq() const { return equal_; }

#ifdef IUMAP_TRACE
  void dump(std::ostream& os) const {
    std::cout << "size=" << size_ << '\n';
    for (auto index = std::size_t{0}; auto const& slot : v_) {
      os << '[' << index << "] ";
      ++index;
      switch (slot.state) {
      case state::unused: os << '*'; break;
      case state::tombstone: os << "\xF0\x9F\xAA\xA6"; break;  // UTF-8 U+1fAA6 tombstone
      case state::occupied: {
        auto const* const kvp = slot.cast();
        os << "> " << kvp->first << '=' << kvp->second;
      } break;
      }
      os << '\n';
    }
  }
#endif  // IUMAP_TRACE

private:
  enum class state : std::uint8_t { occupied, tombstone, unused };
  struct member {
    member() noexcept = default;
    member(member const& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
    member(member&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
    ~member() noexcept { this->destroy(); }
    member& operator=(member const& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
    member& operator=(member&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);

    void destroy() noexcept;

    [[nodiscard]] constexpr value_type* pointer() noexcept { return std::bit_cast<value_type*>(&storage[0]); }
    [[nodiscard]] constexpr value_type const* pointer() const noexcept {
      return std::bit_cast<value_type const*>(&storage[0]);
    }
    [[nodiscard]] constexpr value_type& reference() noexcept { return *this->pointer(); }
    [[nodiscard]] constexpr value_type const& reference() const noexcept { return *this->pointer(); }

    enum state state = state::unused;
    alignas(value_type) std::byte storage[sizeof(value_type)]{};
  };
  [[no_unique_address]] hasher hash_;
  [[no_unique_address]] key_equal equal_;
  std::size_t size_ = 0;
  std::size_t tombstones_ = 0;
  std::array<member, Size> v_{};

#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
  constexpr auto* lookup_slot(this auto& self, Key const& key);
  constexpr auto* find_insert_slot(this auto& self, Key const& key);
#else
  constexpr member* lookup_slot(Key const& key) {
    auto const* const self = this;
    return const_cast<member*>(self->lookup_slot(key));
  }
  constexpr member const* lookup_slot(Key const& key) const;

  constexpr member* find_insert_slot(Key const& key) {
    auto const* const self = this;
    return const_cast<member*>(self->find_insert_slot(key));
  }
  constexpr member const* find_insert_slot(Key const& key) const;
#endif
};

// ctor
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
iumap<Key, Mapped, Size, Hash, KeyEqual>::member::member(member const& other) noexcept(
    std::is_nothrow_copy_constructible_v<value_type>)
    : state{other.state} {
  if (state == state::occupied) {
    std::construct_at(this->pointer(), *other.pointer());
  }
}
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
iumap<Key, Mapped, Size, Hash, KeyEqual>::member::member(member&& other) noexcept(
    std::is_nothrow_move_constructible_v<value_type>)
    : state{other.state} {
  if (state == state::occupied) {
    std::construct_at(this->pointer(), std::move(*other.pointer()));
  }
}

// operator=
// ~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::member::operator=(member const& other) noexcept(
    std::is_nothrow_copy_constructible_v<value_type>) -> member& {
  if (this == &other) {
    return *this;
  }
  if (this->state == state::occupied) {
    this->destroy();
  }
  if (other.state == state::occupied) {
    std::construct_at(this->pointer(), other.reference());
  }
  state = other.state;
  return *this;
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::member::operator=(member&& other) noexcept(
    std::is_nothrow_move_constructible_v<value_type>) -> member& {
  if (this == &other) {
    return *this;
  }
  if (this->state == state::occupied) {
    this->destroy();
  }
  if (other.state == state::occupied) {
    std::construct_at(this->pointer(), std::move(other.reference()));
  }
  state = other.state;
  return *this;
}

// destroy
// ~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
void iumap<Key, Mapped, Size, Hash, KeyEqual>::member::destroy() noexcept {
  if (state == state::occupied) {
    std::destroy_at(this->pointer());
  }
  state = state::unused;
}

// clear
// ~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
void iumap<Key, Mapped, Size, Hash, KeyEqual>::clear() noexcept {
  for (auto& entry : v_) {
    entry.destroy();
  }
  size_ = 0;
  tombstones_ = 0;
}

// try emplace
// ~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename... Args>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::try_emplace(Key const& key, Args&&... args)
    -> std::pair<iterator, bool> {
  auto* const slot = this->find_insert_slot(key);
  if (slot == nullptr) {
    // The map is full and the key was not found. Insertion failed.
    return std::make_pair(this->end(), false);
  }
  auto const do_insert = slot->state != state::occupied;
  if (do_insert) {
    // Not found. Add a new key/value pair.
    new (slot->pointer()) value_type(key, std::forward<Args>(args)...);
    ++size_;
    if (slot->state == state::tombstone) {
      --tombstones_;
    }
    slot->state = state::occupied;
  }
  return {iterator{slot, &v_}, do_insert};
}

// insert
// ~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::insert(value_type const& value) -> std::pair<iterator, bool> {
  return try_emplace(value.first, value.second);
}

// insert or assign
// ~~~~~~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename M>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::insert_or_assign(Key const& key, M&& value)
    -> std::pair<iterator, bool> {
  auto* const slot = this->find_insert_slot(key);
  if (slot == nullptr) {
    // The map is full and the key was not found. Insertion failed.
    return std::make_pair(this->end(), false);
  }
  if (slot->state == state::unused || slot->state == state::tombstone) {
    // Not found. Add a new key/value pair.
    new (slot->storage) value_type(key, std::forward<M>(value));
    ++size_;
    if (slot->state == state::tombstone) {
      --tombstones_;
    }
    slot->state = state::occupied;
    return std::make_pair(iterator{slot, &v_}, true);
  }
  slot->pointer()->second = std::forward<M>(value);  // Overwrite the existing value.
  return {iterator{slot, &v_}, false};
}

// find
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
constexpr auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find(Key const& k) const -> const_iterator {
  auto* const slot = this->lookup_slot(k);
  if (slot == nullptr || slot->state != state::occupied) {
    return this->end();  // Not found
  }
  return {slot, &v_};  // Found
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
constexpr auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find(Key const& k) -> iterator {
  auto* const slot = this->lookup_slot(k);
  if (slot == nullptr || slot->state != state::occupied) {
    return this->end();  // Not found
  }
  return {slot, &v_};  // Found
}

// erase
// ~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::erase(iterator pos) -> iterator {
  member* const slot = pos.raw();
  auto const result = pos + 1;
  if (slot->state == state::occupied) {
    assert(size_ > 0);
    slot->destroy();
    slot->state = state::tombstone;
    --size_;
    ++tombstones_;
    if (this->empty()) {
      this->clear();
    }
  }
  return result;
}

/// Searches the container for a specified key. Stops when the key is found or an unused slot is probed.
/// Tombstones are ignored.
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
constexpr auto* iumap<Key, Mapped, Size, Hash, KeyEqual>::lookup_slot(this auto& self, Key const& key) {
#else
constexpr auto iumap<Key, Mapped, Size, Hash, KeyEqual>::lookup_slot(Key const& key) const -> member const* {
  auto const& self = *this;
#endif  // __cpp_explicit_this_parameter
  using slot_type = std::remove_pointer_t<decltype(v_.data())>;
  using slot_ptr =
      std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, slot_type const*, slot_type*>;

  auto const size = self.v_.size();
  auto pos = self.hash_(key) % size;
  for (auto iteration = 1U; iteration <= size; ++iteration) {
    switch (auto* const slot = &self.v_[pos]; slot->state) {
    case state::unused: return slot;
    case state::tombstone:
      // Keep searching.
      break;
    case state::occupied:
      if (self.equal_(slot->pointer()->first, key)) {
        return slot_ptr{slot};
      }
      break;
    default: assert(false && "Slot is in an invalid state"); break;
    }
    pos = (pos + iteration) % size;
  }
  return slot_ptr{nullptr};  // No available slot.
}

/// Searches the container for a key or a potential insertion position for that key. It stops when either the
/// key or an unused slot are found. If tombstones are encountered, then returns the first tombstone slot
/// so that when inserted, the key's probing distance is as short as possible.
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
constexpr auto* iumap<Key, Mapped, Size, Hash, KeyEqual>::find_insert_slot(this auto& self, Key const& key) {
#else
constexpr auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find_insert_slot(Key const& key) const -> member const* {
  auto const& self = *this;
#endif  // __cpp_explicit_this_parameter
  using slot_type = std::remove_pointer_t<decltype(v_.data())>;
  using slot_ptr =
      std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, slot_type const*, slot_type*>;
  auto const size = self.v_.size();

  auto pos = self.hash_(key) % size;  // The probing position.
  auto* first_tombstone = slot_ptr{nullptr};
  for (auto iteration = 1U; iteration <= size; ++iteration) {
    switch (slot_type* const slot = &self.v_[pos]; slot->state) {
    case state::tombstone:
      if (first_tombstone == nullptr) {
        // Remember this tombstone's slot so it can be returned later.
        first_tombstone = slot;
      }
      break;
    case state::occupied:
      if (self.equal_(slot->pointer()->first, key)) {
        return slot_ptr{slot};
      }
      break;
    case state::unused: return first_tombstone != nullptr ? first_tombstone : slot;
    default: assert(false && "Slot is in an invalid state"); break;
    }
    // The next quadratic probing location
    pos = (pos + iteration) % size;
  }
  return first_tombstone != nullptr ? first_tombstone : slot_ptr{nullptr};
}

}  // end namespace draw

#endif  // DRAW_IUMAP_HPP
