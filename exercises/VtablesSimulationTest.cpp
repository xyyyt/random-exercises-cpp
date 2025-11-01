#include <tuple>
#include <stdexcept>
#include <gtest/gtest.h>

class Object
{
public :
    // simulate virtual member function without parameter
    [[nodiscard]]
    int func() noexcept
    {
        if (std::get<0>(_virtualTables))
        {
            return std::get<0>(_virtualTables)(this);
        }

        return 1;
    }

    // simulate virtual member function with parameter
    [[nodiscard]]
    int func2(int param) noexcept
    {
        if (std::get<1>(_virtualTables))
        {
            return std::get<1>(_virtualTables)(this, param);
        }

        return 2;
    }

    // simulate pure virtual member function without parameter
    [[nodiscard]]
    int func3()
    {
        auto& f = std::get<2>(_virtualTables);

        if (!f)
        {
            throw std::runtime_error("func3 not set");
        }

        return f(this);
    }

protected :
    using VirtualTables_t = std::tuple<
        int (*)(void *),
        int (*)(void *, int),
        int (*)(void *)>;

    VirtualTables_t _virtualTables;
};

class SubObject : public Object
{
public :
    SubObject()
    {
        std::get<0>(_virtualTables) = &staticFunc;
        std::get<1>(_virtualTables) = &staticFunc2;
        std::get<2>(_virtualTables) = &staticFunc3;
    }

    [[nodiscard]]
    int func() noexcept { return 4; }

    [[nodiscard]]
    int func2(int) noexcept { return 5; }

    [[nodiscard]]
    int func3() { return 6; }

private :
    static int staticFunc(void *This)
    {
        return static_cast<SubObject *>(This)->func();
    }

    [[nodiscard]]
    static int staticFunc2(void *This, int param)
    {
        return static_cast<SubObject *>(This)->func2(param);
    }

    static int staticFunc3(void *This)
    {
        return static_cast<SubObject *>(This)->func3();
    }
};

TEST(VtablesSimulation, Test_1)
{
    Object o;

    EXPECT_EQ(o.func(), 1);
    EXPECT_EQ(o.func2(42), 2);
    EXPECT_THROW({ [[maybe_unused]] auto _ = o.func3(); },
                 std::runtime_error);
}

TEST(VTablesSimulation, Test_2)
{
    SubObject o;

    EXPECT_EQ(o.func(), 4);
    EXPECT_EQ(o.func2(42), 5);
    EXPECT_EQ(o.func3(), 6);

    Object& o2 = o;

    EXPECT_EQ(o2.func(), 4);
    EXPECT_EQ(o2.func2(42), 5);
    EXPECT_EQ(o2.func3(), 6);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
