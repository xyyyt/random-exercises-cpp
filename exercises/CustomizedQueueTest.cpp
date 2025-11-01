#include <stack>
#include <memory>
#include <utility>
#include <stdexcept>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace
{
    /* Customized queue using only stack data structure by
       reversing elements order when pop() or front() */
    template <typename T>
    class CustomizedQueue
    {
    public :
        [[nodiscard]]
        inline bool empty() const noexcept
        {
            return _readingStack.empty() && _writingStack.empty();
        }

        [[nodiscard]]
        inline size_t size() const noexcept
        {
            return _readingStack.size() + _writingStack.size();
        }

        template <typename... Args>
        void emplace(Args&&... args)
        {
            _writingStack.emplace(std::make_unique<T>(std::forward<Args>(args)...));
            _needSetReadingStack = true;
        }

        void pop()
        {
            if (empty())
            {
                throw std::runtime_error("stacks are empty");
            }

            if (_needSetReadingStack)
            {
                setReadingStack();
            }

            _readingStack.pop();
        }

        [[nodiscard]]
        const T& front() const
        {
            if (empty())
            {
                throw std::runtime_error("stacks are empty");
            }

            if (_needSetReadingStack)
            {
                setReadingStack();
            }

            return *_readingStack.top();
        }

        [[nodiscard]]
        T& front()
        {
            if (empty())
            {
                throw std::runtime_error("stacks are empty");
            }

            if (_needSetReadingStack)
            {
                setReadingStack();
            }

            return *_readingStack.top();
        }

    private :
        mutable std::stack<std::unique_ptr<T>> _readingStack;
        mutable std::stack<std::unique_ptr<T>> _writingStack;
        mutable bool _needSetReadingStack = false;

        void setReadingStack() const
        {
            auto moveElementsInto = [](
                std::stack<std::unique_ptr<T>>& stack,
                std::stack<std::unique_ptr<T>>& stack2)
            {
                while (!stack2.empty())
                {
                    auto& valuePtr = stack2.top();

                    stack.emplace(std::move(valuePtr));
                    stack2.pop();
                }
            };
            std::stack<std::unique_ptr<T>> newReadingStack;
            std::stack<std::unique_ptr<T>> reversedReadingStack;

            moveElementsInto(newReadingStack, _writingStack);
            moveElementsInto(reversedReadingStack, _readingStack);
            moveElementsInto(newReadingStack, reversedReadingStack);
            _readingStack = std::move(newReadingStack);
            _needSetReadingStack = false;
        }
    };
}

using testing::StrictMock;

TEST(CustomizedQueue, Test_1)
{
    CustomizedQueue<int> cq;

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);

    cq.emplace(1);
    cq.emplace(2);
    cq.emplace(3);
    cq.emplace(4);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 4);
    EXPECT_EQ(cq.front(), 1);

    cq.pop();

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 3);
    EXPECT_EQ(cq.front(), 2);

    cq.pop();

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 2);
    EXPECT_EQ(cq.front(), 3);

    cq.pop();

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 1);
    EXPECT_EQ(cq.front(), 4);

    cq.pop();

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] auto& _ = cq.front(); },
                 std::runtime_error);
    EXPECT_THROW({ cq.pop(); }, std::runtime_error);

    const auto& cq2 = cq;

    ASSERT_TRUE(cq2.empty());
    ASSERT_EQ(cq2.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] const auto& _ = cq2.front(); },
                 std::runtime_error);
}

TEST(CustomizedQueue, Test_2)
{
    CustomizedQueue<int> cq;

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] auto& _ = cq.front(); },
                 std::runtime_error);
    EXPECT_THROW({ cq.pop(); }, std::runtime_error);

    cq.emplace(1);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 1);
    EXPECT_EQ(cq.front(), 1);

    cq.pop();

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] auto& _ = cq.front(); },
                 std::runtime_error);
    EXPECT_THROW({ cq.pop(); }, std::runtime_error);

    cq.emplace(2);
    cq.emplace(3);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 2);
    EXPECT_EQ(cq.front(), 2);

    cq.pop();

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 1);
    EXPECT_EQ(cq.front(), 3);

    cq.emplace(4);
    cq.emplace(5);
    cq.emplace(6);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 4);
    EXPECT_EQ(cq.front(), 3);

    const auto& cq2 = cq;

    cq.pop();

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 3);
    EXPECT_EQ(cq2.front(), 4);

    cq.pop();

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 2);
    EXPECT_EQ(cq2.front(), 5);

    cq.pop();

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 1);
    EXPECT_EQ(cq2.front(), 6);

    cq.pop();

    ASSERT_TRUE(cq2.empty());
    ASSERT_EQ(cq2.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] const auto& _ = cq2.front(); },
                 std::runtime_error);
}

class Mock
{
public :
    MOCK_METHOD(void, onCopyConstructor, (), (const, noexcept));
    MOCK_METHOD(void, onMoveConstructor, (), (const, noexcept));
    MOCK_METHOD(void, onCopyAssignment, (), (const, noexcept));
    MOCK_METHOD(void, onMoveAssignment, (), (const, noexcept));
};

class TestObject
{
public :
    static StrictMock<Mock> *_mockPtr;

    TestObject(int value) noexcept : _value(value) { }

    TestObject(const TestObject&)
    {
        if (_mockPtr)
        {
            _mockPtr->onCopyConstructor();
        }
    }

    TestObject(TestObject&&)
    {
        if (_mockPtr)
        {
            _mockPtr->onMoveConstructor();
        }
    }

    TestObject& operator=(const TestObject&)
    {
        if (_mockPtr)
        {
            _mockPtr->onCopyAssignment();
        }

        return *this;
    }

    TestObject& operator=(TestObject&&)
    {
        if (_mockPtr)
        {
            _mockPtr->onCopyAssignment();
        }

        return *this;
    }

    inline int getValue() const noexcept { return _value; }

private :
    int _value;
};

StrictMock<Mock> *TestObject::_mockPtr = nullptr;

class CustomizedQueueFixture : public testing::Test
{
protected :
    StrictMock<Mock> _mock;

    void SetUp() override { TestObject::_mockPtr = &_mock; }
    void TearDown() override { TestObject::_mockPtr = nullptr; }
};

TEST_F(CustomizedQueueFixture, Test_3)
{
    CustomizedQueue<TestObject> cq;

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] auto& _ = cq.front(); },
                 std::runtime_error);
    EXPECT_THROW({ cq.pop(); }, std::runtime_error);

    cq.emplace(1);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 1);
    EXPECT_EQ(cq.front().getValue(), 1);

    cq.pop();

    ASSERT_TRUE(cq.empty());
    ASSERT_EQ(cq.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] auto& _ = cq.front(); },
                 std::runtime_error);
    EXPECT_THROW({ cq.pop(); }, std::runtime_error);

    cq.emplace(2);
    cq.emplace(3);
    cq.emplace(4);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 3);
    EXPECT_EQ(cq.front().getValue(), 2);

    cq.pop();

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 2);
    EXPECT_EQ(cq.front().getValue(), 3);

    cq.emplace(5);

    ASSERT_FALSE(cq.empty());
    ASSERT_EQ(cq.size(), 3);
    EXPECT_EQ(cq.front().getValue(), 3);

    const auto& cq2 = cq;

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 3);
    EXPECT_EQ(cq2.front().getValue(), 3);

    cq.pop();

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 2);
    EXPECT_EQ(cq2.front().getValue(), 4);

    cq.pop();

    ASSERT_FALSE(cq2.empty());
    ASSERT_EQ(cq2.size(), 1);
    EXPECT_EQ(cq2.front().getValue(), 5);

    cq.pop();

    ASSERT_TRUE(cq2.empty());
    ASSERT_EQ(cq2.size(), 0);
    EXPECT_THROW({ [[maybe_unused]] const auto& _ = cq2.front(); },
                 std::runtime_error);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
