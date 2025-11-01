#include <vector>
#include <type_traits>
#include <gtest/gtest.h>

template <typename Container, typename Functor>
class LazyEvaluation
{
    using ContainerIterator_t = typename Container::const_iterator;

public :
    class Iterator
    {
    public :
        using Value_t = typename Container::value_type;

        inline Iterator() = default;
        inline Iterator(const Iterator&) = default;

        inline Iterator(ContainerIterator_t it, Functor f) noexcept :
            _it(it), _f(f)
        { }

        [[nodiscard]]
        friend inline constexpr bool operator==(
            const Iterator& lhs, const Iterator& rhs) noexcept
        {
            return lhs._it == rhs._it;
        }

        [[nodiscard]]
        friend inline constexpr bool operator!=(
            const Iterator& lhs, const Iterator& rhs) noexcept
        {
            return !operator==(lhs, rhs);
        }

        [[nodiscard]]
        Value_t operator*() const noexcept(
            noexcept(std::declval<Functor>()(std::declval<ContainerIterator_t>())))
        {
            return _f(*_it);
        }

        Iterator& operator++() noexcept
        {
            ++_it;

            return *this;
        }

        [[nodiscard]]
        Iterator operator++(int) noexcept
        {
            Iterator it(*this);

            ++_it;

            return it;
        }

    private :
        ContainerIterator_t _it;
        Functor _f;
    };

    inline LazyEvaluation(const Container& c, Functor f) noexcept :
        _c(c), _f(f)
    { }

    [[nodiscard]]
    inline Iterator begin() const noexcept
    {
        return Iterator(_c.cbegin(), _f);
    }

    [[nodiscard]]
    inline Iterator end() const noexcept
    {
        return Iterator(_c.cend(), _f);
    }

private :
    const Container& _c;
    Functor _f;
};

TEST(LazyEvaluation, Test_1)
{
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto f = [](auto v) noexcept -> int { return v * 2; };
    LazyEvaluation lazyEvaluation(v, f);
    auto begin = lazyEvaluation.begin();
    auto end = lazyEvaluation.end();

    EXPECT_EQ(*begin++, 2);
    EXPECT_EQ(*begin++, 4);
    EXPECT_EQ(*begin++, 6);
    EXPECT_EQ(*begin++, 8);
    EXPECT_EQ(*begin++, 10);
    EXPECT_EQ(begin, end);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
