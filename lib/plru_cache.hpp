#ifndef PLRU_HPP
#define PLRU_HPP

#include <array>
#include <bit>
#include <bitset>
#include <cstddef>
#include <functional>

// Total cache entries == NumSets * NumWays
template <typename Key, typename T, std::size_t NumSets, std::size_t NumWays>
  requires(std::is_unsigned_v<Key> && std::popcount(NumSets) == 1 && std::popcount(NumWays) == 1)
class plru_cache {
public:
  using key_type = Key;
  using mapped_type = T;

  template <typename MissFn, typename EvictFn>
    requires(std::is_invocable_r_v<T, MissFn> && std::is_invocable_v<EvictFn, T &>)
  mapped_type &access(Key const &key, MissFn miss, EvictFn evict) {
    return sets_[key & (NumSets - 1U)].access(key >> index_bits_, miss, evict);
  }

private:
  class tree_plru {
  public:
    void access(std::size_t way) {
      auto node = std::size_t{0};
      auto start = std::size_t{0};
      auto end = NumWays;

      while (node < NumWays - 1U) {
        auto const mid = (start + end) / 2U;
        auto const go_left = way < mid;
        if (go_left) {
          end = mid;
        } else {
          start = mid;
        }
        plru_bits_[node] = go_left;
        node = 2U * node + 1U + static_cast<unsigned>(!go_left);
      }
    }

    std::size_t get_victim() const {
      auto node = std::size_t{0};
      while (node < NumWays - 1U) {
        node = 2U * node + 1U + static_cast<unsigned>(plru_bits_[node]);
      }
      return node - (NumWays - 1U);
    }

  private:
    std::bitset<NumWays - 1> plru_bits_{};
  };

  static constexpr std::size_t index_bits_ = std::bit_width(NumSets - 1);

  class cache_set {
  public:
    template <typename MissFn, typename EvictFn> mapped_type &access(std::size_t tag, MissFn miss, EvictFn evict) {
      // Linear search.
      auto const new_value = tvs{.valid = true, .tag = tag};
      for (auto ctr = std::size_t{0}; ctr < NumWays; ++ctr) {
        if (tag_and_valid_[ctr] == new_value) {
          plru_.access(ctr);
          return ways_[ctr];
        }
      }

      std::size_t const victim = plru_.get_victim();
      if (tag_and_valid_[victim].valid) {
        // If this slot is occupied, call the evict function on it.
        evict(std::ref(ways_[victim]));
      }

      // The key was not found: call miss() to populate it.
      ways_[victim] = miss();
      tag_and_valid_[victim] = new_value;
      plru_.access(victim);
      return ways_[victim];
    }

  private:
    std::array<T, NumWays> ways_{};

    struct tvs {
      friend constexpr bool operator==(tvs const &, tvs const &) noexcept = default;
      std::size_t valid : 1;
      std::size_t : plru_cache::index_bits_ - 1;
      std::size_t tag : sizeof(std::size_t) - plru_cache::index_bits_;
    };
    static_assert(sizeof(tvs) == sizeof(std::size_t));
    std::array<tvs, NumWays> tag_and_valid_{};
    static_assert(sizeof(tag_and_valid_) == 4 * sizeof(std::size_t));

    tree_plru plru_{};
  };

  std::array<cache_set, NumSets> sets_{};
};

#endif  // PLRU_HPP
