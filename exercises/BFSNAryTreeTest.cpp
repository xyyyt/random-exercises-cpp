#include <memory>
#include <queue>
#include <type_traits>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::ElementsAreArray;

namespace
{
    template <typename T>
    struct TreeNode
    {
        T value;
        std::vector<std::shared_ptr<TreeNode<T>>> nodes;

        TreeNode(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            value(value)
        { }
    };

    template <typename T, typename InputIterator>
    void fillQueue(
        std::queue<std::shared_ptr<TreeNode<T>>>& queue,
        InputIterator current,
        InputIterator end)
    {
        if (current == end)
        {
            return;
        }

        queue.emplace(*current);
        fillQueue(queue, ++current, end);
    }

    template <typename T, typename Functor>
    void _bfs(std::queue<std::shared_ptr<TreeNode<T>>>& queue, Functor functor)
    {
        if (queue.empty())
        {
            return;
        }

        std::shared_ptr<TreeNode<T>> current = queue.front();

        queue.pop();
        functor(current->value);
        fillQueue(queue, current->nodes.cbegin(), current->nodes.cend());
        _bfs(queue, functor);
    }

    template <typename T, typename Functor>
    void bfs(const std::shared_ptr<TreeNode<T>>& root, Functor functor)
    {
        std::queue<std::shared_ptr<TreeNode<T>>> queue;

        queue.emplace(root);
        _bfs(queue, functor);
    }

    template <typename T>
    using Expectations_t = std::initializer_list<T>;

    class MyFixture : public testing::Test
    {
    protected :
        template <typename T>
        void check(const std::shared_ptr<TreeNode<T>>& root,
                   Expectations_t<T> expectations) const
        {
            std::vector<T> elementsRoute;

            auto func = [&elementsRoute](const T& value) -> void
            {
                elementsRoute.emplace_back(value);
            };

            bfs(root, func);
            EXPECT_THAT(elementsRoute, ElementsAreArray(expectations));
        }
    };
}

TEST_F(MyFixture, Test_1)
{
    auto root = std::make_shared<TreeNode<int>>(42);
    auto leftNode = std::make_shared<TreeNode<int>>(5);
    auto rightNode = std::make_shared<TreeNode<int>>(460);

    root->nodes.emplace_back(leftNode);
    root->nodes.emplace_back(rightNode);

    auto leftLeftNode = std::make_shared<TreeNode<int>>(9);

    leftNode->nodes.emplace_back(leftLeftNode);

    auto rightLeftNode = std::make_shared<TreeNode<int>>(74);

    rightNode->nodes.emplace_back(rightLeftNode);

    auto rightRightNode = std::make_shared<TreeNode<int>>(314);

    rightNode->nodes.emplace_back(rightRightNode);

    check(root, {42, 5, 460, 9, 74, 314});
}

TEST_F(MyFixture, Test_2)
{
    auto root = std::make_shared<TreeNode<int>>(-214);
    auto child = std::make_shared<TreeNode<int>>(-674);
    auto child2 = std::make_shared<TreeNode<int>>(35);
    auto child3 = std::make_shared<TreeNode<int>>(74);
    auto child4 = std::make_shared<TreeNode<int>>(-57);

    root->nodes.emplace_back(child);
    root->nodes.emplace_back(child2);
    root->nodes.emplace_back(child3);
    root->nodes.emplace_back(child4);

    auto child5 = std::make_shared<TreeNode<int>>(6214);
    auto child6 = std::make_shared<TreeNode<int>>(-41214);
    auto child7 = std::make_shared<TreeNode<int>>(-12222);

    child3->nodes.emplace_back(child5);
    child3->nodes.emplace_back(child6);
    child3->nodes.emplace_back(child7);

    auto child8 = std::make_shared<TreeNode<int>>(-9);

    child7->nodes.emplace_back(child8);

    check(root, {-214, -674, 35, 74, -57, 6214, -41214, -12222, -9});
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
