#include <gtest/gtest.h>
#include <string>
#include <cmath>
#include <climits>
#include <stack>
#include <type_traits>

using Uint_t = unsigned int;

namespace
{
    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, std::string>
    decimalToBinary(T decimal)
    {
        std::stack<char> stack;

        for (uint32_t bitNumber = 0; bitNumber < sizeof(T) * CHAR_BIT; ++bitNumber)
        {
            uint8_t bit = decimal & 1;

            stack.emplace((bit == 0) ? '0' : '1');
            decimal >>= 1;
        }

        std::string binary;

        binary.reserve(stack.size());

        while (!stack.empty())
        {
            auto v = stack.top();

            stack.pop();
            binary.push_back(v);
        }

        return binary;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
    binaryToDecimal(std::string_view binary) noexcept
    {
        T decimal = 0;
        int index = binary.size() - 1;

        for (uint32_t exponent = 0; index > 0; ++exponent)
        {
            decimal += (binary[index--] - 48) * std::pow(2, exponent);
        }

        return decimal;
    }
}

TEST(NaiveDecimalToBinary, Test_decimalToBinary)
{
    EXPECT_EQ(decimalToBinary(Uint_t(42)), "00000000000000000000000000101010");
    EXPECT_EQ(decimalToBinary(Uint_t(6)), "00000000000000000000000000000110");
    EXPECT_EQ(decimalToBinary(Uint_t(125)), "00000000000000000000000001111101");
    EXPECT_EQ(decimalToBinary(Uint_t(0)), "00000000000000000000000000000000");
    EXPECT_EQ(decimalToBinary(Uint_t(31)), "00000000000000000000000000011111");
    EXPECT_EQ(decimalToBinary(Uint_t(3)), "00000000000000000000000000000011");
    EXPECT_EQ(decimalToBinary(Uint_t(98)), "00000000000000000000000001100010");
    EXPECT_EQ(decimalToBinary(Uint_t(3125)), "00000000000000000000110000110101");
    EXPECT_EQ(decimalToBinary(Uint_t(672)), "00000000000000000000001010100000");
}

TEST(NaiveDecimalToBinary, Test_binaryToDecimal)
{
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000101010"), 42);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000000110"), 6);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000001111101"), 125);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000000000"), 0);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000011111"), 31);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000000011"), 3);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000001100010"), 98);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000110000110101"), 3125);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000001010100000"), 672);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
