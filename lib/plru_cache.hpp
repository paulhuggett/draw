/// Implements a "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container. It is
/// intended as a small cache for objects which are relatively cheap to store and relatively expensive
/// to create. The container's keys must be unsigned integral types.
#ifndef DRAW_PLRU_HPP
#define DRAW_PLRU_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <functional>
#include <new>
#include <numeric>

namespace draw {

namespace details {

template <typename T> struct aligned_storage {
  [[nodiscard]] constexpr T &value() noexcept { return *std::bit_cast<T *>(&v[0]); }
  [[nodiscard]] constexpr T const &value() const noexcept { return *std::bit_cast<T const *>(&v[0]); }
  alignas(T) std::byte v[sizeof(T)];
};

template <std::size_t Ways> class tree {
public:
  /// Flip the access bits of the tree to indicate that \p way is the most recently used member.
  void touch(std::size_t const way) noexcept {
    assert(way < Ways && "Way index is too large");
    auto node = std::size_t{0};
    auto start = std::size_t{0};
    auto end = Ways;
    while (node < Ways - 1U) {
      auto const mid = std::midpoint(start, end);
      auto const is_less = way < mid;
      if (is_less) {
        end = mid;
      } else {
        start = mid;
      }
      bits_[node] = is_less;
      node = 2U * node + 1U + static_cast<unsigned>(!is_less);
    }
  }

  /// Traverses the tree to find the index of the oldest member.
  [[nodiscard]] constexpr std::size_t oldest() const noexcept {
    auto node = std::size_t{0};
    while (node < Ways - 1U) {
      node = 2U * node + 1U + static_cast<unsigned>(bits_[node]);
    }
    return node - (Ways - 1U);
  }

private:
  std::bitset<Ways - 1U> bits_{};
};

template <typename MappedType, std::size_t IndexBits, std::size_t Ways>
requires(IndexBits < sizeof(std::size_t) * CHAR_BIT - 1)
class cache_set {
public:
  template <typename MissFn>
  requires(std::is_invocable_r_v<MappedType, MissFn>)
  MappedType &access(std::size_t tag, MissFn miss) {
    assert(tag < (~std::size_t{0} >> IndexBits));
    auto const new_tag = tvs{true, tag};
    // Linear search. The "Ways" argument should be small enough that tags_ fits within a cache line or two.
    for (auto ctr = std::size_t{0}; ctr < Ways; ++ctr) {
      if (tags_[ctr] == new_tag) {
        plru_.touch(ctr);
        return ways_[ctr].value();
      }
    }

    // Find the array member that is to be re-used by traversing the tree.
    std::size_t const victim = plru_.oldest();
    // If this slot is occupied, evict its contents
    if (tags_[victim].valid()) {
      ways_[victim].value().~MappedType();
    }

    // The key was not found: call miss() to populate it.
    auto *const result = new (&ways_[victim].v[0]) MappedType{miss()};
    tags_[victim] = new_tag;
    plru_.touch(victim);
    return *result;
  }

  constexpr auto size() const noexcept {
    return static_cast<std::size_t>(std::ranges::count_if(tags_, [](tvs const &v) { return v.valid(); }));
  }

private:
  struct tvs {
  public:
    constexpr tvs() noexcept = default;
    constexpr tvs(bool valid, std::size_t tag) noexcept : v_{static_cast<std::size_t>(valid) | (tag << 1)} {
      assert(tag <= (~std::size_t{0} >> IndexBits));
    }
    friend constexpr bool operator==(tvs const &, tvs const &) noexcept = default;
    constexpr bool valid() const noexcept { return static_cast<bool>(v_ & 1); }
    constexpr std::size_t tag() const noexcept { return v_ >> 1; }

  private:
    std::size_t v_ = 0;
  };

  std::array<aligned_storage<MappedType>, Ways> ways_;
  std::array<tvs, Ways> tags_{};
  details::tree<Ways> plru_{};
};

}  // end namespace details

/// A "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container. It is
/// intended as a small cache for objects which are relatively cheap to store and relatively
/// expensive to create. The container's keys must be unsigned integral types.
///
/// The total number of cache entries is given by Sets * Ways.
///
/// \tparam Sets  The number of entries that share the same lookup key fragment or hash bucket
///   index. All entries in a set compete to be stored in that group.
/// \tparam Ways  The number of slots within a set that can hold a single entry. The number of
///   ways in a set determines how many entries with the same key fragment or bucket index can
///   coexist.
template <typename Key, typename T, std::size_t Sets, std::size_t Ways>
requires(std::is_unsigned_v<Key> && std::popcount(Sets) == 1 && std::popcount(Ways) == 1)
class plru_cache {
public:
  using key_type = Key;
  using mapped_type = T;

  /// Searches the cache for the \p key and returns a reference to it if present. If not present
  /// and the cache is fully occupied, the likely least recently used value is evicted from the
  /// cache. The \p miss function is called to instantiate the mapped type associated with \p key :
  /// this value is then stored in the cache.
  ///
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache.
  /// \returns The cached value.
  template <typename MissFn>
  requires(std::is_invocable_r_v<T, MissFn>)
  mapped_type &access(Key const &key, MissFn miss) {
    return sets_[key & (Sets - 1U)].access(key >> index_bits_, miss);
  }

  constexpr std::size_t max_size() const noexcept { return Sets * Ways; }
  constexpr std::size_t size() const noexcept {
    return std::ranges::fold_left(sets_, std::size_t{0},
                                  [](std::size_t acc, auto const &set) { return acc + set.size(); });
  }

private:
  static constexpr std::size_t index_bits_ = std::bit_width(Sets - 1U);
  std::array<details::cache_set<T, index_bits_, Ways>, Sets> sets_{};
};

} // end namespace draw

#endif  // PLRU_HPP
