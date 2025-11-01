#include <gtest/gtest.h>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <string_view>

namespace
{
    template <typename ForwardIterator, typename T>
    [[nodiscard]]
    bool binarySearch(ForwardIterator begin, ForwardIterator end, const T& value) noexcept
    {
        using Difference = typename std::iterator_traits<ForwardIterator>::difference_type;

        Difference distance = std::distance(begin, end);

        if (!distance)
        {
            return false;
        }

        ForwardIterator it = begin;
        uint32_t n = 0;

        for (uint32_t n = 0; n < distance / 2; ++n)
        {
            it = std::next(it);
        }

        if (*it == value)
        {
            return true;
        }
        else if (*it > value)
        {
            return binarySearch(begin, it, value);
        }
        else
        {
            return binarySearch(std::next(it), end, value);
        }
    }

    struct Empty { };

    template <typename>
    class BinarySearchWithSeqContainerFixture;

    template <template <typename...> class T, typename... Args>
    class BinarySearchWithSeqContainerFixture<T<Args...>> : public testing::Test
    {
    protected :
        template <typename Value, typename... Args2>
        using SeqContainer = T<Value, Args2...>;
    };

    template <typename>
    class BinarySearchWithAssocContainerFixture;

    template <template <typename...> class T, typename... Args>
    class BinarySearchWithAssocContainerFixture<T<Args...>> : public testing::Test
    {
    protected :
        template <typename Key, typename Value, typename... Args2>
        using AssocContainer = T<Key, Value, Args2...>;
    };

    class NameGenerator
    {
    public :
        template <typename T>
        static std::string GetName(int)
        {
            if constexpr (std::is_same_v<T, std::vector<Empty>>)
            {
                return "vector";
            }
            else if constexpr (std::is_same_v<T, std::deque<Empty>>)
            {
                return "deque";
            }
            else if constexpr (std::is_same_v<T, std::list<Empty>>)
            {
                return "list";
            }
            else if constexpr (std::is_same_v<T, std::deque<Empty>>)
            {
                return "set";
            }
            else if constexpr (std::is_same_v<T, std::unordered_set<Empty>>)
            {
                return "unordered_set";
            }
            else if constexpr (std::is_same_v<T, std::map<Empty, Empty>>)
            {
                return "map";
            }
            else if constexpr (std::is_same_v<T, std::unordered_map<Empty, Empty>>)
            {
                return "unordered_map";
            }
            else
            {
                testing::KilledBySignal(SIGABRT);
            }

            return "";
        }
    };
}

TYPED_TEST_SUITE_P(BinarySearchWithSeqContainerFixture);

TYPED_TEST_P(BinarySearchWithSeqContainerFixture, TestBinarySearchWithSeqContainer_1)
{
    using SeqContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::SeqContainer<int>;

    const SeqContainer ELEMS = {1, 2, 3, 4};

    if (!std::is_sorted(ELEMS.cbegin(), ELEMS.cend()))
    {
        GTEST_SKIP();
    }

    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 1));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 2));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 3));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 4));

    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -32));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 79));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 56));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -238));
}

TYPED_TEST_P(BinarySearchWithSeqContainerFixture, TestBinarySearchWithSeqContainer_2)
{
    using SeqContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::SeqContainer<int>;

    const SeqContainer ELEMS = {95, -9, 32, 17, -42};

    if (!std::is_sorted(ELEMS.cbegin(), ELEMS.cend()))
    {
        GTEST_SKIP();
    }

    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 95));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -9));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 32));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 17));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -42));

    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 0));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 4212));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -875));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), 14));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), -74));
}

REGISTER_TYPED_TEST_SUITE_P(BinarySearchWithSeqContainerFixture,
                            TestBinarySearchWithSeqContainer_1,
                            TestBinarySearchWithSeqContainer_2);

using MySeqContainerTypes = testing::Types<
    std::vector<Empty>,
    std::deque<Empty>,
    std::list<Empty>,
    std::set<Empty>,
    std::unordered_set<Empty>>;

INSTANTIATE_TYPED_TEST_SUITE_P(MySeqContainerInstantiation,
                               BinarySearchWithSeqContainerFixture,
                               MySeqContainerTypes,
                               NameGenerator);


TYPED_TEST_SUITE_P(BinarySearchWithAssocContainerFixture);

TYPED_TEST_P(BinarySearchWithAssocContainerFixture, TestBinarySearchWithAssocContainer_1)
{
    using AssocContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::AssocContainer<int, std::string_view>;
    using Pair = typename AssocContainer::value_type;

    const AssocContainer ELEMS = {{1, "abcd"}, {2, "efgh"}, {3, "ijkl"}, {4, "mnop"}};

    if (!std::is_sorted(ELEMS.cbegin(), ELEMS.cend()))
    {
        GTEST_SKIP();
    }

    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(1, "abcd")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(2, "efgh")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(3, "ijkl")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(4, "mnop")));

    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-32, "ngrz")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(79, "tqse")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(56, "agra")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-238, "pfqs")));
}

TYPED_TEST_P(BinarySearchWithAssocContainerFixture, TestBinarySearchWithAssocContainer_2)
{
    using AssocContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::AssocContainer<int, std::string_view>;
    using Pair = typename AssocContainer::value_type;

    const AssocContainer ELEMS = {
        {95, "nsqd"}, {-9, "tafg"}, {32, "rars"}, {17, "heds"}, {-42, "qsbr"}};

    if (!std::is_sorted(ELEMS.cbegin(), ELEMS.cend()))
    {
        GTEST_SKIP();
    }

    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(95, "nsqd")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-9, "tafg")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(32, "rars")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(17, "heds")));
    EXPECT_TRUE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-42, "qsbr")));

    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(0, "qsvg")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(4212, "yzta")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-875, "wwdq")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(14, "tart")));
    EXPECT_FALSE(binarySearch(ELEMS.cbegin(), ELEMS.cend(), Pair(-74, "ktez")));
}

REGISTER_TYPED_TEST_SUITE_P(BinarySearchWithAssocContainerFixture,
                            TestBinarySearchWithAssocContainer_1,
                            TestBinarySearchWithAssocContainer_2);

using MyAssocContainerTypes = testing::Types<
    std::map<Empty, Empty>, std::unordered_map<Empty, Empty>>;

INSTANTIATE_TYPED_TEST_SUITE_P(MyAssocContainerInstantiation,
                               BinarySearchWithAssocContainerFixture,
                               MyAssocContainerTypes,
                               NameGenerator);

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
