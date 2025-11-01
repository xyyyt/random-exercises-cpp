#include <memory>
#include <type_traits>
#include <vector>
#include <stack>
#include <queue>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::ElementsAreArray;

namespace
{
    enum class ReverseTreeMode : uint8_t
    {
        RECURSIVE_PRE_ORDER,
        RECURSIVE_POST_ORDER,
        ITERATIVE_WITH_STACK,
        ITERATIVE_WITH_QUEUE
    };

    template <typename T>
    struct TreeNode
    {
        T value;
        std::shared_ptr<TreeNode<T>> left;
        std::shared_ptr<TreeNode<T>> right;

        TreeNode(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            value(value)
        { }
    };

    [[nodiscard]]
    constexpr const char *toString(ReverseTreeMode mode) noexcept
    {
        switch (mode)
        {
            case ReverseTreeMode::RECURSIVE_PRE_ORDER :
                return "RecursivePreOrder";
            case ReverseTreeMode::RECURSIVE_POST_ORDER :
                return "RecursivePostOrder";
            case ReverseTreeMode::ITERATIVE_WITH_STACK :
                return "IterativeWithStack";
            case ReverseTreeMode::ITERATIVE_WITH_QUEUE :
                return "IterativeWithQueue";
        }

        return "";
    }

    template <typename T>
    void recursivePreOrder_reverseTree(std::shared_ptr<TreeNode<T>>& node) noexcept
    {
        if (!node)
        {
            return;
        }

        std::swap(node->left, node->right);
        recursivePreOrder_reverseTree(node->left);
        recursivePreOrder_reverseTree(node->right);
    }

    template <typename T>
    void recursivePostOrder_reverseTree(std::shared_ptr<TreeNode<T>>& node) noexcept
    {
        if (!node)
        {
            return;
        }

        recursivePostOrder_reverseTree(node->left);
        recursivePostOrder_reverseTree(node->right);
        std::swap(node->left, node->right);
    }

    template <typename T>
    void iterativeWithStack_reverseTree(std::shared_ptr<TreeNode<T>>& root)
    {
        if (!root)
        {
            return;
        }

        std::stack<std::shared_ptr<TreeNode<T>>> nodes;

        nodes.emplace(root);

        while (!nodes.empty())
        {
            auto current = nodes.top();

            nodes.pop();
            std::swap(current->left, current->right);

            if (current->left)
            {
                nodes.emplace(current->left);
            }

            if (current->right)
            {
                nodes.emplace(current->right);
            }
        }
    }

    template <typename T>
    void iterativeWithQueue_reverseTree(std::shared_ptr<TreeNode<T>>& root)
    {
        if (!root)
        {
            return;
        }

        std::queue<std::shared_ptr<TreeNode<T>>> nodes;

        nodes.emplace(root);

        while (!nodes.empty())
        {
            auto current = nodes.front();

            nodes.pop();
            std::swap(current->left, current->right);

            if (current->left)
            {
                nodes.emplace(current->left);
            }

            if (current->right)
            {
                nodes.emplace(current->right);
            }
        }
    }

    class MyFixture :
        public testing::Test,
        public testing::WithParamInterface<ReverseTreeMode>
    {
    protected :
        template <typename T>
        void apply(std::shared_ptr<TreeNode<T>>& root,
                   ReverseTreeMode mode) const
        {
            switch (mode)
            {
                case ReverseTreeMode::RECURSIVE_PRE_ORDER :
                    recursivePreOrder_reverseTree(root);

                    break;
                case ReverseTreeMode::RECURSIVE_POST_ORDER :
                    recursivePostOrder_reverseTree(root);

                    break;
                case ReverseTreeMode::ITERATIVE_WITH_STACK :
                    iterativeWithStack_reverseTree(root);

                    break;
                case ReverseTreeMode::ITERATIVE_WITH_QUEUE :
                    iterativeWithQueue_reverseTree(root);

                    break;
            }
        }

        template <typename T>
        void check(const std::shared_ptr<TreeNode<T>>& root,
                   std::initializer_list<T> expectations) const
        {
            std::vector<T> elementsRoute;

            fillElementsRoute(root, elementsRoute);
            EXPECT_THAT(elementsRoute, ElementsAreArray(expectations));
        }

    private :
        template <typename T>
        void fillElementsRoute(const std::shared_ptr<TreeNode<T>>& node,
                               std::vector<T>& elementsRoute) const
        {
            if (!node)
            {
                return;
            }

            elementsRoute.emplace_back(node->value);
            fillElementsRoute(node->left, elementsRoute);
            fillElementsRoute(node->right, elementsRoute);
        }
    };

    class MyFixtureParametrizedTest_1 : public MyFixture { };
    class MyFixtureParametrizedTest_2 : public MyFixture { };
}

TEST_P(MyFixtureParametrizedTest_1, Test)
{
    ReverseTreeMode mode = GetParam();

        //       1
        //     /   \
        //    /     \
        //   2       5
        //  / \     / \
        // 3   4   6   7

    auto root = std::make_shared<TreeNode<int>>(1);

    root->left = std::make_shared<TreeNode<int>>(2);
    root->right = std::make_shared<TreeNode<int>>(5);

    root->left->left = std::make_shared<TreeNode<int>>(3);
    root->left->right = std::make_shared<TreeNode<int>>(4);

    root->right->left = std::make_shared<TreeNode<int>>(6);
    root->right->right = std::make_shared<TreeNode<int>>(7);

    apply(root, mode);

        //       1
        //     /   \
        //    /     \
        //   5       2
        //  / \     / \
        // 7   6   4   3

    check(root, {1, 5, 7, 6, 2, 4, 3});
}

TEST_P(MyFixtureParametrizedTest_2, Test)
{
    ReverseTreeMode mode = GetParam();

        //       1
        //         \
        //          \
        //           2
        //          / \
        //         3   4
        //              \
        //               5

    auto root = std::make_shared<TreeNode<int>>(1);

    root->right = std::make_shared<TreeNode<int>>(2);

    root->right->left = std::make_shared<TreeNode<int>>(3);
    root->right->right = std::make_shared<TreeNode<int>>(4);

    root->right->right->right = std::make_shared<TreeNode<int>>(5);

    apply(root, mode);
    check(root, {1, 2, 4, 5, 3});
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixtureParametrizedTest_1,
    testing::Values(
           ReverseTreeMode::RECURSIVE_PRE_ORDER,
           ReverseTreeMode::RECURSIVE_POST_ORDER,
           ReverseTreeMode::ITERATIVE_WITH_STACK,
           ReverseTreeMode::ITERATIVE_WITH_QUEUE),
    [](const testing::TestParamInfo<MyFixtureParametrizedTest_1::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "With_" << toString(info.param);

        return oss.str();
    });

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixtureParametrizedTest_2,
    testing::Values(
        ReverseTreeMode::RECURSIVE_PRE_ORDER,
        ReverseTreeMode::RECURSIVE_POST_ORDER,
        ReverseTreeMode::ITERATIVE_WITH_STACK,
        ReverseTreeMode::ITERATIVE_WITH_QUEUE),
    [](const testing::TestParamInfo<MyFixtureParametrizedTest_2::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "With_" << toString(info.param);

        return oss.str();
    });

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
