#include <type_traits>
#include <string>
#include <sstream>
#include <gtest/gtest.h>

namespace
{
    template <typename Integral, typename = std::enable_if_t<std::is_integral_v<Integral>>>
    class WrappedIntegral
    {
    public :
        WrappedIntegral(Integral n) noexcept : _n(n) { }
        WrappedIntegral(const WrappedIntegral&) = default;
        WrappedIntegral(WrappedIntegral&&) = default;
        ~WrappedIntegral() = default;
        WrappedIntegral& operator=(const WrappedIntegral&) = default;
        WrappedIntegral& operator=(WrappedIntegral&&) = default;

        [[nodiscard]]
        constexpr bool operator==(const WrappedIntegral& other) const noexcept
        {
            return _n == other._n;
        }

        [[nodiscard]]
        constexpr bool operator!=(const WrappedIntegral& other) const noexcept
        {
            return _n != other._n;
        }

        [[nodiscard]]
        constexpr bool operator<(const WrappedIntegral& other) const noexcept
        {
            return _n < other._n;
        }

        [[nodiscard]]
        constexpr bool operator>(const WrappedIntegral& other) const noexcept
        {
            return _n > other._n;
        }

        [[nodiscard]]
        constexpr bool operator<=(const WrappedIntegral& other) const noexcept
        {
            return _n <= other._n;
        }

        [[nodiscard]]
        constexpr bool operator>=(const WrappedIntegral& other) const noexcept
        {
            return _n >= other._n;
        }

        inline WrappedIntegral& operator++() noexcept
        {
            ++_n;

            return *this;
        }

        inline WrappedIntegral operator++(Integral) noexcept
        {
            WrappedIntegral old(*this);

            operator++();

            return old;
        }

        inline WrappedIntegral& operator--() noexcept
        {
            --_n;

            return *this;
        }

        inline WrappedIntegral operator--(Integral) noexcept
        {
            WrappedIntegral old(*this);

            operator--();

            return old;
        }

        [[nodiscard]] inline operator bool() const noexcept { return _n != 0; }
        [[nodiscard]] inline operator std::string() const { return std::to_string(_n); }
        [[nodiscard]] inline operator Integral() const noexcept { return Integral(_n); }

    private :
        Integral _n;

        friend std::ostream& operator<<(std::ostream& os, const WrappedIntegral& iwo)
        {
            os << iwo._n;

            return os;
        }
    };
}

TEST(TestWrappedIntegral, Test_1)
{
    using Integral = int;

    const WrappedIntegral<Integral> VALUE_TO_COMPARE = 1;
    WrappedIntegral<Integral> iwo(0);

    {
        EXPECT_FALSE(iwo == VALUE_TO_COMPARE);
        EXPECT_TRUE(iwo != VALUE_TO_COMPARE);
        EXPECT_TRUE(iwo < VALUE_TO_COMPARE);
        EXPECT_FALSE(iwo > VALUE_TO_COMPARE);
        EXPECT_TRUE(iwo <= VALUE_TO_COMPARE);
        EXPECT_FALSE(iwo >= VALUE_TO_COMPARE);
    }

    {
        ++iwo;
        EXPECT_EQ(Integral(iwo), 1);

        WrappedIntegral<Integral> old = iwo++;

        EXPECT_EQ(Integral(old), 1);
        EXPECT_EQ(Integral(iwo), 2);

        --iwo;
        EXPECT_EQ(Integral(iwo), 1);

        WrappedIntegral<Integral> old2 = iwo--;

        EXPECT_EQ(Integral(old2), 1);
        EXPECT_EQ(Integral(iwo), 0);
    }

    {
        EXPECT_FALSE(bool(iwo));
        EXPECT_EQ(std::string(iwo), "0");

        std::ostringstream oss;

        oss << iwo;
        EXPECT_EQ(oss.str(), "0");
    }
}

TEST(TestWrappedIntegral, Test_2)
{
    using Integral = int;

    const WrappedIntegral<Integral> VALUE_TO_COMPARE = 42;
    WrappedIntegral<Integral> iwo(42);

    {
        EXPECT_TRUE(iwo == VALUE_TO_COMPARE);
        EXPECT_FALSE(iwo != VALUE_TO_COMPARE);
        EXPECT_FALSE(iwo < VALUE_TO_COMPARE);
        EXPECT_FALSE(iwo > VALUE_TO_COMPARE);
        EXPECT_TRUE(iwo <= VALUE_TO_COMPARE);
        EXPECT_TRUE(iwo >= VALUE_TO_COMPARE);
    }

    {
        ++iwo;
        EXPECT_EQ(Integral(iwo), 43);

        WrappedIntegral<Integral> old = iwo++;

        EXPECT_EQ(Integral(old), 43);
        EXPECT_EQ(Integral(iwo), 44);

        --iwo;
        EXPECT_EQ(Integral(iwo), 43);

        WrappedIntegral<Integral> old2 = iwo--;

        EXPECT_EQ(Integral(old2), 43);
        EXPECT_EQ(Integral(iwo), 42);
    }

    {
        EXPECT_TRUE(bool(iwo));
        EXPECT_EQ(std::string(iwo), "42");

        std::ostringstream oss;

        oss << iwo;
        EXPECT_EQ(oss.str(), "42");
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
