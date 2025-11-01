#include <sstream>
#include <tuple>
#include <gtest/gtest.h>

namespace
{
    class MyFixture : public testing::TestWithParam<std::tuple<bool, int>> { };
}

TEST_P(MyFixture, TestBoolIsTrueAndIntegerIsNotNegative)
{
    const auto [b, n] = GetParam();

    EXPECT_TRUE(b);
    EXPECT_FALSE(n < 0);
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixture,
    testing::Values(std::make_tuple(true, 3),
                    std::make_tuple(true, 6),
                    std::make_tuple(true, 10)),
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
