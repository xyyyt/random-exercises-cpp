#include <string_view>
#include <string>
#include <tuple>
#include <sstream>
#include <gtest/gtest.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace
{
    void _getReversedString(const char *s, std::string& s2, uint32_t index = 0)
    {
        if (*s == '\0')
        {
            s2.reserve(index);

            return;
        }

        _getReversedString(s + 1, s2, index + 1);
        s2.push_back(*s);
    }

    inline void _getReversedString(const std::string& s, std::string& s2)
    {
        s2.reserve(s.length());

        for (auto rbegin = s.crbegin(); rbegin != s.crend(); ++rbegin)
        {
            s2.push_back(*rbegin);
        }
    }

    void _getReversedString(std::string_view s, std::string& s2, uint32_t index = 0)
    {
        if (s.empty())
        {
            s2.reserve(index);

            return;
        }

        _getReversedString(s.substr(1), s2, index + 1);
        s2.push_back(s[0]);
    }

    template <typename T>
    [[nodiscard]]
    std::string getReversedString(T&& s)
    {
        if (s.empty())
        {
            return "";
        }

        std::string s2;

        _getReversedString(s, s2);

        return s2;
    }

    [[nodiscard]]
    std::string getReversedString(const char *s)
    {
        if (!s)
        {
            return "";
        }

        std::string s2;

        _getReversedString(s, s2);

        return s2;
    }

    class MyFixture : public testing::TestWithParam<std::tuple<
        const char *, const char *>> { };
}

TEST_P(MyFixture, TestGetReversedString)
{
    const auto [parameter, expected] = GetParam();

    EXPECT_EQ(getReversedString(parameter), expected); // const char * parameter
    EXPECT_EQ(getReversedString(std::string(parameter)), expected); // std::string parameter
    EXPECT_EQ(getReversedString(std::string_view(parameter)), expected); // std::string_view parameter
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixture,
    testing::Values(std::make_tuple("abcdefg", "gfedcba"),
                    std::make_tuple("12345", "54321")),
    [](const testing::TestParamInfo<MyFixture::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "ValuesAre_"
            << std::boolalpha
            << std::get<0>(info.param)
            << "_And_"
            << std::get<1>(info.param);

        return oss.str();
    });

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
