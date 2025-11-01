#include <unordered_map>
#include <list>
#include <tuple>
#include <utility>
#include <cstddef>
#include <gtest/gtest.h>

inline void hash_combine(size_t& seed, size_t value) noexcept
{
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

namespace std
{
    template <typename... Args>
    struct hash<std::tuple<Args...>>
    {
        [[nodiscard]]
        size_t operator()(const std::tuple<Args...>& args) const
        {
            size_t seed = 0;

            std::apply([&seed](const Args&... elems) noexcept -> void
            {
                (..., hash_combine(seed, std::hash<Args>{}(elems)));
            }, args);

            return seed;
        }
    };
}

namespace cache
{
    template <size_t, typename>
    class LRUCache;

    template <size_t CAPACITY, typename Ret, typename... Args>
    class LRUCache<CAPACITY, Ret (Args...)>
    {
        static_assert(CAPACITY > 0, "CAPACITY cannot be equal to 0");
        static_assert(!std::is_same_v<void, Ret>, "Return type cannot be void");
        static_assert(sizeof...(Args) > 0, "Args parameters cannot be empty");

    public :
        [[nodiscard]]
        inline bool empty() const noexcept { return _lru.empty(); }

        [[nodiscard]]
        inline bool isFull() const noexcept { return _lru.size() == CAPACITY; }

        void add(const Ret& retValue, Args&&... args)
        {
            std::tuple<Args&&...> tuple(std::forward<Args>(args)...);

            if (auto keysIt = _keys.find(tuple); keysIt != _keys.cend())
            {
                if (keysIt->second != std::prev(_lru.cend()))
                {
                    _lru.splice(_lru.end(), _lru, keysIt->second);

                    auto lruIt = std::prev(_lru.end());

                    _keys[tuple] = lruIt;
                }

                return;
            }

            if (_lru.size() >= CAPACITY)
            {
                _keys.erase(std::get<0>(*_lru.cbegin()));
                _lru.pop_front();
            }

            _lru.emplace_back(std::make_tuple(std::forward<Args>(args)...), retValue);

            auto lruIt = std::prev(_lru.cend());

            _keys[tuple] = lruIt;
        }

        [[nodiscard]]
        const Ret *get(Args&&... args) const
        {
            if (_lru.empty())
            {
                return nullptr;
            }

            std::tuple<Args&&...> tuple(std::forward<Args>(args)...);
            auto keysIt = _keys.find(tuple);

            if (keysIt == _keys.cend())
            {
                return nullptr;
            }
            else if (keysIt->second == std::prev(_lru.cend()))
            {
                return &keysIt->second->second;
            }

            _lru.splice(_lru.end(), _lru, keysIt->second);

            auto lruIt = std::prev(_lru.end());

            _keys[tuple] = lruIt;

            return &lruIt->second;
        }

        void clear()
        {
            _lru.clear();
            _keys.clear();
        }

    private :
        mutable std::list<
            std::pair<std::tuple<Args...>, Ret>> _lru;
        mutable std::unordered_map<
            std::tuple<Args...>, typename decltype(_lru)::const_iterator> _keys;
    };

    template <size_t, typename, typename>
    class CachedFunction;

    template <size_t CAPACITY, typename Ret, typename... Args, typename Function>
    class CachedFunction<CAPACITY, Ret (Args...), Function>
    {
    public :
        inline CachedFunction(Function function) noexcept :
            _function(function)
        { }

        [[nodiscard]]
        inline bool cacheIsFull() const noexcept { return _cache.isFull(); }

        [[nodiscard]]
        inline bool lastCallFromCache() const noexcept { return _lastCallFromCache; }

        [[nodiscard]]
        Ret operator()(Args&&... args)
        {
            if (auto ptr = _cache.get(std::forward<Args>(args)...); ptr)
            {
                _lastCallFromCache = true;

                return *ptr;
            }

            auto retValue = _function(std::forward<Args>(args)...);

            _cache.add(retValue, std::forward<Args>(args)...);
            _lastCallFromCache = false;

            return retValue;
        }

        inline void clearCache() { _cache.clear(); }

    private :
        LRUCache<CAPACITY, Ret (Args...)> _cache;
        Function _function;
        bool _lastCallFromCache = false;
    };
};

TEST(CachedFunction, Test_1)
{
    using namespace cache;

    auto f = [](int n) noexcept -> int { return n + 2; };
    CachedFunction<1, int (int), decltype(f)> cachedFunc(f);

    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(1), 3);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(1), 3);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(2), 4);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(2), 4);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    cachedFunc.clearCache();
    EXPECT_FALSE(cachedFunc.cacheIsFull());
}

TEST(CachedFunction, Test_2)
{
    using namespace cache;

    auto f = [](int n) noexcept -> int { return n * n; };
    CachedFunction<2, int (int), decltype(f)> cachedFunc(f);

    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(2), 4);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(2), 4);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(4), 16);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(4), 16);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(6), 36);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(6), 36);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_TRUE(cachedFunc.cacheIsFull());

    cachedFunc.clearCache();
    EXPECT_FALSE(cachedFunc.cacheIsFull());
}

TEST(CachedFunction, Test_3)
{
    using namespace cache;

    auto f = [](float f, float f2) noexcept -> float { return f * f2; };
    CachedFunction<5, float (float, float), decltype(f)> cachedFunc(f);

    EXPECT_EQ(cachedFunc(2.5, 5.0), 12.5);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(2.5, 5.0), 12.5);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(5.0, 2.5), 12.5);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(5.0, 2.5), 12.5);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(10.0, 4.0), 40);
    EXPECT_FALSE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    EXPECT_EQ(cachedFunc(10.0, 4.0), 40);
    EXPECT_TRUE(cachedFunc.lastCallFromCache());
    EXPECT_FALSE(cachedFunc.cacheIsFull());

    cachedFunc.clearCache();
    EXPECT_FALSE(cachedFunc.cacheIsFull());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
