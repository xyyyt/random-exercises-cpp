#include <unordered_set>
#include <string>
#include <string_view>
#include <tuple>
#include <gtest/gtest.h>

namespace
{
    [[nodiscard]]
    std::string removeDuplicatedCharacters(std::string_view s)
    {
        std::unordered_set<char> charactersAlreadyMet;
        std::string str;

        str.reserve(s.size());

        for (char c : s)
        {
            if (charactersAlreadyMet.find(c) == charactersAlreadyMet.cend())
            {
                charactersAlreadyMet.emplace(c);
                str.push_back(c);
            }
        }

        return str;
    }

    class MyFixture : public testing::TestWithParam<std::tuple<
        const char *, const char *>> { };
}

TEST_P(MyFixture, TestRemoveDuplicatedCharacters)
{
    const auto [parameter, expected] = GetParam();

    EXPECT_EQ(removeDuplicatedCharacters(parameter), expected);
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixture,
    testing::Values(std::make_tuple("abcd", "abcd"),
                    std::make_tuple("aabbccdd", "abcd"),
                    std::make_tuple("abcdabcd", "abcd")));

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
