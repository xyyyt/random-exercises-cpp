#include <string>
#include <type_traits>
#include <gtest/gtest.h>

namespace
{
    template <typename>
    class MyFixture;

    template <>
    class MyFixture<int> : public testing::Test
    {
    protected :
        static constexpr int _VALUE = 42;
    };

    template <>
    class MyFixture<char> : public testing::Test
    {
    protected :
        static constexpr char _VALUE = 'c';
    };

    template <>
    class MyFixture<std::string> : public testing::Test
    {
    protected :
        static const std::string _VALUE;
    };

    const std::string MyFixture<std::string>::_VALUE = "hello";

    class NameGenerator
    {
    public :
        template <typename T>
        [[nodiscard]]
        static std::string GetName(int)
        {
            if constexpr (std::is_same_v<T, int>)
            {
                return "int";
            }
            else if constexpr (std::is_same_v<T, char>)
            {
                return "char";
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return "std::string";
            }
            else
            {
                testing::KilledBySignal(SIGABRT);
            }
        }
    };
}

TYPED_TEST_SUITE_P(MyFixture);

TYPED_TEST_P(MyFixture, TestIsEqualToSpecificValue)
{
    auto&& value = this->_VALUE;

    if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>)
    {
        EXPECT_EQ(value, 42);
    }
    else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, char>)
    {
        EXPECT_EQ(value, 'c');
    }
    else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>)
    {
        EXPECT_EQ(value, "hello");
    }
    else
    {
        FAIL() << "Type not expected";
    }
}

REGISTER_TYPED_TEST_SUITE_P(MyFixture, TestIsEqualToSpecificValue);

using MyTypes = testing::Types<int, char, std::string>;

INSTANTIATE_TYPED_TEST_SUITE_P(MyInstantiation,
                               MyFixture,
                               MyTypes,
                               NameGenerator);

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
