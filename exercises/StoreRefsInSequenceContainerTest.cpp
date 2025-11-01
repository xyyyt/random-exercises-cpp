#include <gtest/gtest.h>
#include <functional>
#include <variant>
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace
{
    // overload pattern
    template <typename... Functor>
    struct Overloaded : Functor... { using Functor::operator()...; };

    template <typename... Functor>
    Overloaded(Functor...) -> Overloaded<Functor...>;

    struct Empty { };

    template <typename>
    class StoreRefsInSequenceContainerFixture;

    template <typename... Args>
    class StoreRefsInSequenceContainerFixture<std::vector<Args...>> : public testing::Test
    {
    protected :
        template <typename... Args2>
        using SequenceContainer = std::vector<Args2...>;
    };

    template <typename... Args>
    class StoreRefsInSequenceContainerFixture<std::deque<Args...>> : public testing::Test
    {
    protected :
        template <typename... Args2>
        using SequenceContainer = std::deque<Args2...>;
    };

    template <typename... Args>
    class StoreRefsInSequenceContainerFixture<std::forward_list<Args...>> : public testing::Test
    {
    protected :
        template <typename... Args2>
        using SequenceContainer = std::forward_list<Args2...>;
    };

    template <typename... Args>
    class StoreRefsInSequenceContainerFixture<std::list<Args...>> : public testing::Test
    {
    protected :
        template <typename... Args2>
        using SequenceContainer = std::list<Args2...>;
    };

    template <typename T, size_t N>
    class StoreRefsInSequenceContainerFixture<std::array<T, N>> : public testing::Test
    {
    protected :
        template <typename T2,
                  size_t N2 = 3 /* Tricks to compile tests with 3 elements only */>
        using SequenceContainer = std::array<T2, N2>;
    };

    class NameGenerator
    {
    public :
        template <typename T>
        [[nodiscard]]
        static std::string GetName(int)
        {
            if constexpr (std::is_same_v<T, std::array<Empty, 0>>)
            {
                return "array";
            }
            else if constexpr (std::is_same_v<T, std::vector<Empty>>)
            {
                return "vector";
            }
            else if constexpr (std::is_same_v<T, std::deque<Empty>>)
            {
                return "deque";
            }
            else if constexpr (std::is_same_v<T, std::forward_list<Empty>>)
            {
                return "forward_list";
            }
            else if constexpr (std::is_same_v<T, std::list<Empty>>)
            {
                return "list";
            }
            else
            {
                testing::KilledBySignal(SIGABRT);
            }
        }
    };
}

TYPED_TEST_SUITE_P(StoreRefsInSequenceContainerFixture);

TYPED_TEST_P(StoreRefsInSequenceContainerFixture, TestWithVariant)
{
    using RefInt = std::reference_wrapper<int>;
    using RefFloat = std::reference_wrapper<float>;
    using RefString = std::reference_wrapper<std::string>;
    using Variant = std::variant<RefInt, RefFloat, RefString>;
    using SequenceContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::SequenceContainer<Variant>;

    constexpr Overloaded OVERLOADED =
    {
        [](RefInt& refInt) -> void
        {
            int& value = refInt.get();

            EXPECT_EQ(value, 84);
            value = 126;
        },
        [](RefFloat& refFloat) -> void
        {
            float& value = refFloat.get();

            EXPECT_EQ(value, 51.0);
            value = 76.5;
        },
        [](RefString& refString) -> void
        {
            std::string& value = refString.get();

            EXPECT_EQ(value, "Goodbye World !");
            value = "Still World !";
        }
    };
    int i = 42;
    float f = 25.5;
    std::string s = "Hello World !";
    SequenceContainer container{i, f, s};

    i = 84;
    f = 51.0;
    s = "Goodbye World !";

    for (auto& variant : container)
    {
        std::visit(OVERLOADED, variant);
    }

    EXPECT_EQ(i, 126);
    EXPECT_EQ(f, 76.5);
    EXPECT_EQ(s, "Still World !");
}

TYPED_TEST_P(StoreRefsInSequenceContainerFixture, TestWithAny)
{
    using RefInt = std::reference_wrapper<int>;
    using RefFloat = std::reference_wrapper<float>;
    using RefString = std::reference_wrapper<std::string>;
    using SequenceContainer = typename std::decay_t<
        std::remove_pointer_t<decltype(this)>>::SequenceContainer<std::any>;

    const std::unordered_map<std::type_index, void (*)(std::any&)> VISITORS =
    {
        {
            typeid(RefInt),
            [](std::any& any) -> void
            {
                int& value = std::any_cast<RefInt>(any).get();

                EXPECT_EQ(value, 84);
                value = 126;
            }
        },
        {
            typeid(RefFloat),
            [](std::any& any) -> void
            {
                float& value = std::any_cast<RefFloat>(any).get();

                EXPECT_EQ(value, 51.0);
                value = 76.5;
            }
        },
        {
            typeid(RefString),
            [](std::any& any) -> void
            {
                std::string& value = std::any_cast<RefString>(any).get();

                EXPECT_EQ(value, "Goodbye World !");
                value = "Still World !";
            }
        }
    };
    int i = 42;
    float f = 25.5;
    std::string s = "Hello World !";
    SequenceContainer container{std::ref(i), std::ref(f), std::ref(s)};

    i = 84;
    f = 51.0;
    s = "Goodbye World !";

    for (auto& any : container)
    {
        VISITORS.at(any.type())(any);
    }

    EXPECT_EQ(i, 126);
    EXPECT_EQ(f, 76.5);
    EXPECT_EQ(s, "Still World !");
}

REGISTER_TYPED_TEST_SUITE_P(StoreRefsInSequenceContainerFixture,
                            TestWithVariant,
                            TestWithAny);

using MyTypes = testing::Types<
    std::array<Empty, 0>,
    std::vector<Empty>,
    std::deque<Empty>,
    std::forward_list<Empty>,
    std::list<Empty>>;

INSTANTIATE_TYPED_TEST_SUITE_P(MyInstantiation,
                               StoreRefsInSequenceContainerFixture,
                               MyTypes,
                               NameGenerator);

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
