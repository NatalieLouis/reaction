#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
namespace reaction {
  class UniqueID {
   public:
    UniqueID() : m_id(generateID()) {}

    operator uint64_t() const { return m_id; }

    bool operator==(const UniqueID& other) const { return m_id == other.m_id; }

   private:
    uint64_t generateID() {
      static std::atomic<uint64_t> counter{0};
      return counter.fetch_add(1, std::memory_order_relaxed);
    }
    uint64_t m_id;
    friend struct std::hash<UniqueID>;
  };

}  // namespace reaction

// 允许UniqueID作为unordered_set和unordered_map的键
namespace std {
  template <>
  struct hash<reaction::UniqueID> {
    size_t operator()(const reaction::UniqueID& uid) const { return std::hash<uint64_t>()(uid); }
  };
}  // namespace std