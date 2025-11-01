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
    std::enable_if_t<std::is_integral_v<T>, std::string> decimalToBinary(T decimal)
    {
        // Two's complement algorithm for negative value
        if (decimal < 0)
        {
            decimal = ~(decimal * -1) + 1;
        }

        constexpr uint32_t NUMBER_OF_BIT = sizeof(T) * CHAR_BIT;
        std::stack<char> stack;

        for (uint32_t bitNumber = 0; bitNumber < NUMBER_OF_BIT; ++bitNumber)
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
    std::enable_if_t<std::is_integral_v<T>, bool>
    isBinaryWithRightSize(std::string_view binary) noexcept
    {
        constexpr uint32_t NUMBER_OF_BIT = sizeof(T) * CHAR_BIT;

        if (binary.size() != NUMBER_OF_BIT)
        {
            return false;
        }

        for (auto v : binary)
        {
            if (v != '0' && v != '1')
            {
                return false;
            }
        }

        return true;
    }

    template <typename T>
    [[nodiscard]]
    std::enable_if_t<std::is_integral_v<T>, T> binaryToDecimal(std::string_view binary)
    {
        assert(isBinaryWithRightSize<T>(binary));

        bool isSignedTypeAndNegativeValue = std::is_signed_v<T> && binary[0] == '1';
        std::string binaryCopy(binary.data(), binary.size());

        // Two's complement algorithm for negative value by binary reverting
        if (isSignedTypeAndNegativeValue)
        {
            for (auto& v : binaryCopy)
            {
                v = (v == '0') ? '1' : '0';
            }
        }

        constexpr uint32_t NUMBER_OF_BIT = sizeof(T) * CHAR_BIT;
        uint32_t pos = (isSignedTypeAndNegativeValue) ? 1 : 0;
        T decimal = 0;

        for (uint32_t exponent = (isSignedTypeAndNegativeValue) ?
                 NUMBER_OF_BIT - 2 : NUMBER_OF_BIT - 1;
             pos < binary.size();
             --exponent)
        {
            decimal += (binaryCopy[pos++] - 48) * std::pow(2, exponent);
        }

        /* Two's complement algorithm for negative value by adding 1
           and transform new decimal to negative */
        if (isSignedTypeAndNegativeValue)
        {
            decimal = (decimal + 1) * -1;
        }

        return decimal;
    }
}

TEST(ComplementTwoDecimalToBinary, Test_decimalToBinary)
{
    EXPECT_EQ(decimalToBinary(Uint_t(6)), "00000000000000000000000000000110");
    EXPECT_EQ(decimalToBinary(Uint_t(125)), "00000000000000000000000001111101");
    EXPECT_EQ(decimalToBinary(Uint_t(0)), "00000000000000000000000000000000");
    EXPECT_EQ(decimalToBinary(Uint_t(31)), "00000000000000000000000000011111");
    EXPECT_EQ(decimalToBinary(Uint_t(98)), "00000000000000000000000001100010");
    EXPECT_EQ(decimalToBinary(Uint_t(3125)), "00000000000000000000110000110101");

    EXPECT_EQ(decimalToBinary(-42), "11111111111111111111111111010110");
    EXPECT_EQ(decimalToBinary(9), "00000000000000000000000000001001");
    EXPECT_EQ(decimalToBinary(587), "00000000000000000000001001001011");
    EXPECT_EQ(decimalToBinary(-3), "11111111111111111111111111111101");
    EXPECT_EQ(decimalToBinary(21315), "00000000000000000101001101000011");
    EXPECT_EQ(decimalToBinary(7424), "00000000000000000001110100000000");
    EXPECT_EQ(decimalToBinary(4), "00000000000000000000000000000100");
    EXPECT_EQ(decimalToBinary(-672), "11111111111111111111110101100000");
}

TEST(ComplementTwoDecimalToBinary, Test_isBinaryWithRightSize)
{
    EXPECT_TRUE(isBinaryWithRightSize<Uint_t>("00000000000000000000000000000110"));
    EXPECT_TRUE(isBinaryWithRightSize<Uint_t>("00000000000000000000000001111101"));
    EXPECT_TRUE(isBinaryWithRightSize<Uint_t>("00000000000000000000000000000000"));
    EXPECT_TRUE(isBinaryWithRightSize<Uint_t>("00000000000000000000000000011111"));

    EXPECT_TRUE(isBinaryWithRightSize<int>("11111111111111111111111111010110"));
    EXPECT_TRUE(isBinaryWithRightSize<int>("00000000000000000000000000001001"));
    EXPECT_TRUE(isBinaryWithRightSize<int>("00000000000000000000001001001011"));
    EXPECT_TRUE(isBinaryWithRightSize<int>("11111111111111111111111111111101"));

    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("00000000000000011000"));
    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("000000111110100000"));
    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("00"));
    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("0000000000000000000000011111"));
    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("11111121114111161111811111111101"));
    EXPECT_FALSE(isBinaryWithRightSize<Uint_t>("111111c11141111d1111811t11111101"));

    EXPECT_FALSE(isBinaryWithRightSize<int>("1111111111111111111111111010110"));
    EXPECT_FALSE(isBinaryWithRightSize<int>("0000000000000000001001"));
    EXPECT_FALSE(isBinaryWithRightSize<int>("00001001011"));
    EXPECT_FALSE(isBinaryWithRightSize<int>("111101"));
    EXPECT_FALSE(isBinaryWithRightSize<int>("000000000000W00D0000001001001011"));
    EXPECT_FALSE(isBinaryWithRightSize<int>("1111111111111111111111111111110J"));
}

TEST(ComplementTwoDecimalToBinary, Test_binaryToDecimal)
{
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000000110"), 6);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000001111101"), 125);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000000000"), 0);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000000011111"), 31);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000000001100010"), 98);
    EXPECT_EQ(binaryToDecimal<Uint_t>("00000000000000000000110000110101"), 3125);

    EXPECT_EQ(binaryToDecimal<int>("11111111111111111111111111010110"), -42);
    EXPECT_EQ(binaryToDecimal<int>("00000000000000000000000000001001"), 9);
    EXPECT_EQ(binaryToDecimal<int>("00000000000000000000001001001011"), 587);
    EXPECT_EQ(binaryToDecimal<int>("11111111111111111111111111111101"), -3);
    EXPECT_EQ(binaryToDecimal<int>("00000000000000000101001101000011"), 21315);
    EXPECT_EQ(binaryToDecimal<int>("00000000000000000001110100000000"), 7424);
    EXPECT_EQ(binaryToDecimal<int>("00000000000000000000000000000100"), 4);
    EXPECT_EQ(binaryToDecimal<int>("11111111111111111111110101100000"), -672);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
