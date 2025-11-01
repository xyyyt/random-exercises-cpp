#include <unordered_set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace
{
    [[nodiscard]]
    std::unordered_map<char, uint32_t> countOccurrences(std::string_view s)
    {
        std::unordered_map<char, uint32_t> occurrencesCounter;

        for (char c : s)
        {
            ++occurrencesCounter[c];
        }

        auto begin = occurrencesCounter.begin();
        auto end = occurrencesCounter.end();

        while (begin != end)
        {
            if (begin->second < 2)
            {
                begin = occurrencesCounter.erase(begin);
            }
            else
            {
                ++begin;
            }
        }

        return occurrencesCounter;
    }

    class MyFixture : public testing::TestWithParam<std::tuple<
        const char *, std::unordered_map<char, uint32_t>>> { };
}

TEST_P(MyFixture, TestCountOccurrences)
{
    auto&& [parameter, expected] = GetParam();

    EXPECT_THAT(countOccurrences(parameter),
                testing::UnorderedElementsAreArray(expected));
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixture,
    testing::Values(std::make_tuple("abcd", std::unordered_map<char, uint32_t> { }),
                    std::make_tuple("abbcdd", std::unordered_map<char, uint32_t> {
                            {'b', 2},
                            {'d', 2}}),
                    std::make_tuple("abccccd", std::unordered_map<char, uint32_t> {
                            {'c', 4}})));

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
