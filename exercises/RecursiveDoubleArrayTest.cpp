#include <type_traits>
#include <tuple>
#include <gtest/gtest.h>

namespace
{
    template <typename T,
        size_t ROW,
        size_t COLUMN,
        typename = std::enable_if_t<std::is_fundamental_v<T>>>
    class RecursiveDoubleArray
    {
    public :
        RecursiveDoubleArray() { recursiveAlloc(); }
        ~RecursiveDoubleArray() { recursiveDelete(); }

        void fill(T value) noexcept { recursiveFillRows(value); }

        bool set(size_t x, size_t y, T value) noexcept
        {
            if (x >= ROW || y >= COLUMN)
            {
                return false;
            }

            _doubleArray[x][y] = value;

            return true;
        }

        bool get(size_t x, size_t y, T& value) noexcept
        {
            if (x >= ROW || y >= COLUMN)
            {
                return false;
            }

            value = _doubleArray[x][y];

            return true;
        }

        template <typename Functor>
        void forEach(Functor f) noexcept { recursiveForEachRows(f); }

    private :
        /* could be replaced by std::unique_ptr<std::unique_ptr<T[]>[]>,
           but use raw pointers to deliberately complicate the exercice */
        T **_doubleArray = nullptr;

        void recursiveAllocColumns(uint32_t n = 0)
        {
            if (n >= ROW)
            {
                return;
            }

            _doubleArray[n] = new T[COLUMN];
            recursiveFillColumns(T(), n);
            recursiveAllocColumns(n + 1);
        }

        void recursiveAlloc()
        {
            _doubleArray = new T *[ROW];
            recursiveAllocColumns();
        }

        void recursiveDeleteRows(uint32_t n = 0)
        {
            if (n >= ROW)
            {
                return;
            }

            delete[] _doubleArray[n];
            recursiveDeleteRows(n + 1);
        }

        void recursiveDelete()
        {
            recursiveDeleteRows();
            delete[] _doubleArray;
        }

        void recursiveFillColumns(T value, uint32_t n, uint32_t n2 = 0) noexcept
        {
            if (n2 >= COLUMN)
            {
                return;
            }

            _doubleArray[n][n2] = value;
            recursiveFillColumns(value, n, n2 + 1);
        }

        void recursiveFillRows(T value, uint32_t n = 0) noexcept
        {
            if (n >= ROW)
            {
                return;
            }

            recursiveFillColumns(value, n);
            recursiveFillRows(value, n + 1);
        }

        void recursiveFill(T value) noexcept { recursiveFillRows(value); }

        template <typename Functor>
        void recursiveForEachColumns(Functor f, uint32_t n, uint32_t n2 = 0) noexcept
        {
            if (n2 >= COLUMN)
            {
                return;
            }

            f(_doubleArray[n][n2]);
            recursiveForEachColumns(f, n, n2 + 1);
        }

        template <typename Functor>
        void recursiveForEachRows(Functor f, uint32_t n = 0) noexcept
        {
            if (n >= ROW)
            {
                return;
            }

            recursiveForEachColumns(f, n);
            recursiveForEachRows(f, n + 1);
        }
    };

    template <typename>
    struct GetFirstTemplateParam;

    template <template <typename...> class T, typename... Args>
    struct GetFirstTemplateParam<T<Args...>>
    {
        using Type = std::tuple_element_t<0, std::tuple<Args...>>;
    };

    template <typename T>
    class MyFixture : public testing::Test { };
}

using MyTypes = testing::Types<int, char, short, long>;

TYPED_TEST_SUITE(MyFixture, MyTypes);

TYPED_TEST(MyFixture, Test)
{
    using Type = typename GetFirstTemplateParam<
        std::decay_t<std::remove_pointer_t<decltype(this)>>>::Type;

    RecursiveDoubleArray<Type, 5, 3> doubleArray;

    doubleArray.forEach([](Type value) noexcept -> void
    {
        EXPECT_EQ(value, 0);
    });

    doubleArray.fill(42);

    EXPECT_FALSE(doubleArray.set(5, 3, 1));
    EXPECT_FALSE(doubleArray.set(10, 6, 1));

    {
        Type value;

        EXPECT_FALSE(doubleArray.get(5, 3, value));
        EXPECT_FALSE(doubleArray.get(10, 6, value));
    }

    doubleArray.forEach([](Type value) noexcept -> void
    {
        EXPECT_EQ(value, 42);
    });

    EXPECT_TRUE(doubleArray.set(2, 2, 1));

    {
        Type value;

        EXPECT_TRUE(doubleArray.get(2, 2, value));
        EXPECT_EQ(value, 1);
    }

    EXPECT_TRUE(doubleArray.set(4, 0, 19));

    {
        Type value;

        EXPECT_TRUE(doubleArray.get(4, 0, value));
        EXPECT_EQ(value, 19);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
