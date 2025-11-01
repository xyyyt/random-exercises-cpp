#include <gtest/gtest.h>

class MyClass
{
private :
    friend class TestWithFixture;
    FRIEND_TEST(TestWithoutFixture, Sample);
    FRIEND_TEST(TestWithFixture, Sample2);

    [[nodiscard]]
    constexpr int multByFourtyTwo(int n) const noexcept { return n * 42; }
};

class TestWithFixture : public testing::Test
{
protected :
    void runExpectations() const
    {
        MyClass myClass;

        EXPECT_EQ(myClass.multByFourtyTwo(1), 42);
        EXPECT_EQ(myClass.multByFourtyTwo(2), 84);
    }
};

TEST(TestWithoutFixture, Sample)
{
    MyClass myClass;

    EXPECT_EQ(myClass.multByFourtyTwo(1), 42);
    EXPECT_EQ(myClass.multByFourtyTwo(2), 84);
}

TEST_F(TestWithFixture, Sample2)
{
    runExpectations();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
