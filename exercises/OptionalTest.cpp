#include <cstddef>
#include <exception>
#include <gtest/gtest.h>

namespace
{
    class BadOptionalAccess : public std::exception
    {
    public :
        ~BadOptionalAccess() override = default;

        [[nodiscard]]
        const char *what() const noexcept override
        {
            return "bad optional access";
        }
    };

    template <typename T>
    class Optional
    {
    public :
        Optional() = default;
        ~Optional() { destroy(); }

        Optional(const Optional& other)
        {
            _hasValue = other._hasValue;

            if (other._hasValue)
            {
                _element = new (_memory) T(*other._element);
            }
        }

        Optional(Optional&& other)
        {
            _hasValue = other._hasValue;

            if (other._hasValue)
            {
                _element = new (_memory) T(std::move(*other._element));
                other._hasValue = false;
            }
        }

        Optional& operator=(const Optional& other)
        {
            if (this != &other)
            {
                _hasValue = other._hasValue;

                if (_hasValue)
                {
                    *_element = *other._element;
                }
            }

            return *this;
        }

        Optional& operator=(Optional&& other)
        {
            if (this != &other)
            {
                _hasValue = other._hasValue;

                if (_hasValue)
                {
                    *_element = std::move(*other._element);
                    other._hasValue = false;
                }
            }

            return *this;
        }

        [[nodiscard]]
        operator bool() const noexcept { return _hasValue; }

        [[nodiscard]]
        const T *operator->() const noexcept { return _element; }

        [[nodiscard]]
        T *operator->() noexcept { return _element; }

        [[nodiscard]]
        const T& operator*() const noexcept { return *_element; }

        [[nodiscard]]
        T& operator*() noexcept { return *_element; }

        [[nodiscard]]
        const T& value() const
        {
            if (!_hasValue)
            {
                throw BadOptionalAccess();
            }

            return *_element;
        }

        [[nodiscard]]
        T& value()
        {
            if (!_hasValue)
            {
                throw BadOptionalAccess();
            }

            return *_element;
        }

        [[nodiscard]]
        bool hasValue() const noexcept { return operator bool(); }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            _hasValue = true;
            _element = new (_memory) T(std::forward<Args>(args)...);
        }

        void reset() { destroy(); }

    private :
        bool _hasValue = false;
        std::byte _memory[sizeof(T)];
        T *_element = reinterpret_cast<T *>(_memory);

        void destroy()
        {
            if (_hasValue)
            {
                _element->~T();
                _hasValue = false;
            }
        }
    };
}

TEST(Optional, Test_1)
{
    Optional<int> opt;

    EXPECT_FALSE(opt);
    EXPECT_FALSE(opt.hasValue());
    EXPECT_THROW({ [[maybe_unused]] auto _ = opt.value(); },
                 BadOptionalAccess);

    opt.emplace(42);

    EXPECT_TRUE(opt);
    EXPECT_TRUE(opt.hasValue());
    EXPECT_EQ(opt.value(), 42);
    EXPECT_EQ(*opt.operator->(), 42);
    EXPECT_EQ(*opt, 42);

    Optional<int> opt2 = opt;

    opt2.emplace(84);

    EXPECT_TRUE(opt);
    EXPECT_TRUE(opt.hasValue());
    EXPECT_EQ(opt.value(), 42);
    EXPECT_EQ(*opt.operator->(), 42);
    EXPECT_EQ(*opt, 42);

    EXPECT_TRUE(opt2);
    EXPECT_TRUE(opt2.hasValue());
    EXPECT_EQ(opt2.value(), 84);
    EXPECT_EQ(*opt2.operator->(), 84);
    EXPECT_EQ(*opt2, 84);

    opt.reset();

    EXPECT_FALSE(opt);
    EXPECT_FALSE(opt.hasValue());
    EXPECT_THROW({ [[maybe_unused]] auto _ = opt.value(); },
                 BadOptionalAccess);

    Optional<int> opt3 = std::move(opt2);

    EXPECT_FALSE(opt2);
    EXPECT_FALSE(opt2.hasValue());
    EXPECT_THROW({ [[maybe_unused]] auto _ = opt2.value(); },
                 BadOptionalAccess);

    EXPECT_TRUE(opt3);
    EXPECT_TRUE(opt3.hasValue());
    EXPECT_EQ(opt3.value(), 84);
    EXPECT_EQ(*opt3.operator->(), 84);
    EXPECT_EQ(*opt3, 84);
}

TEST(Optional, Test_2)
{
    Optional<int> opt;

    opt.reset();

    EXPECT_FALSE(opt);
    EXPECT_FALSE(opt.hasValue());
    EXPECT_THROW({ [[maybe_unused]] auto _ = opt.value(); },
                 BadOptionalAccess);

    opt.emplace(1);
    opt.reset();
    opt.emplace(2);
    opt.reset();
    opt.emplace(3);
    opt.emplace(4);

    EXPECT_TRUE(opt);
    EXPECT_TRUE(opt.hasValue());
    EXPECT_EQ(opt.value(), 4);
    EXPECT_EQ(*opt.operator->(), 4);
    EXPECT_EQ(*opt, 4);

    Optional<int> opt2;

    opt2.emplace(42);
    opt2.emplace(5);

    EXPECT_TRUE(opt2);
    EXPECT_TRUE(opt2.hasValue());
    EXPECT_EQ(opt2.value(), 5);
    EXPECT_EQ(*opt2.operator->(), 5);
    EXPECT_EQ(*opt2, 5);

    opt2 = opt;

    EXPECT_TRUE(opt2);
    EXPECT_TRUE(opt2.hasValue());
    EXPECT_EQ(opt2.value(), 4);
    EXPECT_EQ(*opt.operator->(), 4);
    EXPECT_EQ(*opt, 4);

    Optional<int> opt3;

    opt3.reset();
    opt3.emplace(6);
    opt3 = std::move(opt2);

    EXPECT_FALSE(opt2);
    EXPECT_FALSE(opt2.hasValue());
    EXPECT_THROW({ [[maybe_unused]] auto _ = opt2.value(); },
                 BadOptionalAccess);

    EXPECT_TRUE(opt3);
    EXPECT_TRUE(opt3.hasValue());
    EXPECT_EQ(opt3.value(), 4);
    EXPECT_EQ(*opt3.operator->(), 4);
    EXPECT_EQ(*opt3, 4);

    const Optional<int> opt4 = opt3;

    EXPECT_TRUE(opt4);
    EXPECT_TRUE(opt4.hasValue());
    EXPECT_EQ(opt4.value(), 4);
    EXPECT_EQ(*opt4.operator->(), 4);
    EXPECT_EQ(*opt4, 4);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
