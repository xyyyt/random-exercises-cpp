#include <memory>
#include <sstream>
#include <type_traits>
#include <tuple>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::ElementsAreArray;

namespace
{
    enum class DFSAlgo : uint8_t
    {
        PRE_ORDER,
        POST_ORDER,
        REVERSED_PRE_ORDER,
        REVERSED_POST_ORDER,
    };

    [[nodiscard]]
    constexpr const char *toString(DFSAlgo algo) noexcept
    {
        switch (algo)
        {
            case DFSAlgo::PRE_ORDER :
                return "DFS_PreOrder";
            case DFSAlgo::POST_ORDER :
                return "DFS_PostOrder";
            case DFSAlgo::REVERSED_PRE_ORDER :
                return "DFS_ReversedPreOrder";
            case DFSAlgo::REVERSED_POST_ORDER :
                return "DFS_ReversedPostOrder";
        }

        return "";
    }

    template <typename T>
    struct TreeNode
    {
        T value;
        std::vector<std::shared_ptr<TreeNode<T>>> nodes;

        TreeNode(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            value(value)
        { }
    };

    template <typename InputIterator, typename Functor>
    void _dfsReversedPostOrder(InputIterator current, InputIterator end, Functor functor)
        noexcept(noexcept(functor((*current)->value)))
    {
        if (current == end)
        {
            return;
        }

        _dfsReversedPostOrder((*current)->nodes.cbegin(), (*current)->nodes.cend(), functor);
        _dfsReversedPostOrder(current + 1, end, functor);
        functor((*current++)->value);
    }

    template <typename T, typename Functor>
    void dfsReversedPostOrder(const std::shared_ptr<TreeNode<T>>& root, Functor functor)
    {
        std::vector<std::shared_ptr<TreeNode<T>>> nodes{root};

        _dfsReversedPostOrder(nodes.cbegin(), nodes.cend(), functor);
    }

    template <typename InputIterator, typename Functor>
    void _dfsReversedPreOrder(InputIterator current, InputIterator end, Functor functor)
        noexcept(noexcept(functor((*current)->value)))
    {
        if (current == end)
        {
            return;
        }

        _dfsReversedPreOrder(current + 1, end, functor);
        functor((*current)->value);
        _dfsReversedPreOrder((*current)->nodes.cbegin(), (*current)->nodes.cend(), functor);
    }

    template <typename T, typename Functor>
    void dfsReversedPreOrder(const std::shared_ptr<TreeNode<T>>& root, Functor functor)
    {
        std::vector<std::shared_ptr<TreeNode<T>>> nodes{root};

        _dfsReversedPreOrder(nodes.cbegin(), nodes.cend(), functor);
    }

    template <typename InputIterator, typename Functor>
    void _dfsPostOrder(InputIterator current, InputIterator end, Functor functor)
        noexcept(noexcept(functor((*current)->value)))
    {
        if (current == end)
        {
            return;
        }

        _dfsPostOrder((*current)->nodes.cbegin(), (*current)->nodes.cend(), functor);
        functor((*current)->value);
        _dfsPostOrder(++current, end, functor);
    }

    template <typename T, typename Functor>
    void dfsPostOrder(const std::shared_ptr<TreeNode<T>>& root, Functor functor)
    {
        std::vector<std::shared_ptr<TreeNode<T>>> nodes{root};

        _dfsPostOrder(nodes.cbegin(), nodes.cend(), functor);
    }

    template <typename InputIterator, typename Functor>
    void _dfsPreOrder(InputIterator current, InputIterator end, Functor functor)
        noexcept(noexcept(functor((*current)->value)))
    {
        if (current == end)
        {
            return;
        }

        functor((*current)->value);
        _dfsPreOrder((*current)->nodes.cbegin(), (*current)->nodes.cend(), functor);
        _dfsPreOrder(++current, end, functor);
    }

    template <typename T, typename Functor>
    void dfsPreOrder(const std::shared_ptr<TreeNode<T>>& root, Functor functor)
    {
        std::vector<std::shared_ptr<TreeNode<T>>> nodes{root};

        _dfsPreOrder(nodes.cbegin(), nodes.cend(), functor);
    }

    template <typename T>
    using Expectations_t = std::vector<T>;

    class MyFixture :
        public testing::Test,
        public testing::WithParamInterface<std::tuple<DFSAlgo, Expectations_t<int>>>
    {
    protected :
        template <typename T>
        void check(const std::shared_ptr<TreeNode<T>>& root,
                   DFSAlgo dfsAlgo,
                   const Expectations_t<T>& expectations) const
        {
            std::vector<T> elementsRoute;

            auto func = [&elementsRoute](const T& value) -> void
            {
                elementsRoute.emplace_back(value);
            };

            switch (dfsAlgo)
            {
                case DFSAlgo::PRE_ORDER :
                    dfsPreOrder(root, func);

                    break;
                case DFSAlgo::POST_ORDER :
                    dfsPostOrder(root, func);

                    break;
                case DFSAlgo::REVERSED_PRE_ORDER :
                    dfsReversedPreOrder(root, func);

                    break;
                case DFSAlgo::REVERSED_POST_ORDER :
                    dfsReversedPostOrder(root, func);

                    break;
            }

            EXPECT_THAT(elementsRoute, ElementsAreArray(expectations));
        }
    };

    class MyFixtureParametrizedTest_1 : public MyFixture { };
    class MyFixtureParametrizedTest_2 : public MyFixture { };
}

TEST_P(MyFixtureParametrizedTest_1, Test)
{
    auto&& [dfsAlgo, expectations] = GetParam();
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

    check(root, dfsAlgo, expectations);
}

TEST_P(MyFixtureParametrizedTest_2, Test)
{
    auto&& [dfsAlgo, expectations] = GetParam();
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

    check(root, dfsAlgo, expectations);
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixtureParametrizedTest_1,
    testing::Values(std::make_tuple(
        DFSAlgo::PRE_ORDER,
        Expectations_t<int>{42, 5, 9, 460, 74, 314}),
                    std::make_tuple(
        DFSAlgo::POST_ORDER,
        Expectations_t<int>{9, 5, 74, 314, 460, 42}),
                    std::make_tuple(
        DFSAlgo::REVERSED_PRE_ORDER,
        Expectations_t<int>{42, 460, 314, 74, 5, 9}),
                    std::make_tuple(
        DFSAlgo::REVERSED_POST_ORDER,
        Expectations_t<int>{9, 314, 74, 460, 5, 42})),
    [](const testing::TestParamInfo<MyFixtureParametrizedTest_1::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "With_" << toString(std::get<0>(info.param));

        return oss.str();
    });

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixtureParametrizedTest_2,
    testing::Values(std::make_tuple(
        DFSAlgo::PRE_ORDER,
        Expectations_t<int>{-214, -674, 35, 74, 6214, -41214, -12222, -9, -57}),
                    std::make_tuple(
        DFSAlgo::POST_ORDER,
        Expectations_t<int>{-674, 35, 6214, -41214, -9, -12222, 74, -57, -214}),
                    std::make_tuple(
        DFSAlgo::REVERSED_PRE_ORDER,
        Expectations_t<int>{-214, -57, 74, -12222, -9, -41214, 6214, 35, -674}),
                    std::make_tuple(
        DFSAlgo::REVERSED_POST_ORDER,
        Expectations_t<int>{-9, -12222, -41214, 6214, -57, 74, 35, -674, -214})),
    [](const testing::TestParamInfo<MyFixtureParametrizedTest_2::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "With_" << toString(std::get<0>(info.param));

        return oss.str();
    });

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
