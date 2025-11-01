#include <utility>
#include <type_traits>
#include <cstddef>
#include <gtest/gtest.h>

namespace
{
    template <typename T, size_t N>
    class StaticBinaryTree
    {
        static_assert(
            std::is_default_constructible_v<T>, "T must be default constructible");
        static_assert(
            N > 0, "N must be greater than 0");

        struct Node;

    public :
        class ContiguousMemoryAccess
        {
        public :
            class Iterator
            {
            public :
                Iterator() noexcept = default;
                Iterator(const Iterator&) noexcept = default;

                [[nodiscard]]
                friend inline bool operator==(
                    const Iterator& lhs, const Iterator& rhs) noexcept
                {
                    return lhs._nodes == rhs._nodes
                        && lhs._index == rhs._index;
                }

                [[nodiscard]]
                friend inline bool operator!=(
                    const Iterator& lhs, const Iterator& rhs) noexcept
                {
                    return !operator==(lhs, rhs);
                }

                Iterator& operator++()
                {
                    assert(_nodes);
                    assert(_index < N);
                    next();

                    return *this;
                }

                [[nodiscard]]
                Iterator operator++(int)
                {
                    assert(_nodes);
                    assert(_index < N);

                    Iterator it = *this;

                    next();

                    return it;
                }

                [[nodiscard]]
                inline T& operator*() noexcept { return _nodes[_index].value; }

            private :
                friend class ContiguousMemoryAccess;

                Node *_nodes = nullptr;
                int _index = 0;

                inline Iterator(Node *nodes, int index = 0) noexcept :
                    _nodes(nodes), _index(index)
                { }

                void next()
                {
                    assert(_index < N);

                    do
                    {
                        ++_index;

                        if (_nodes[_index].index != -1)
                        {
                            break;
                        }
                    }
                    while (_index < N);
                }
            };

            [[nodiscard]]
            inline Iterator begin() noexcept
            {
                return Iterator(_nodes, (_size > 0) ? 0 : N);
            }

            [[nodiscard]]
            inline Iterator end() noexcept
            {
                return Iterator(_nodes, N);
            }

        private :
            friend class StaticBinaryTree;

            Node (&_nodes)[N];
            size_t& _size;

            inline ContiguousMemoryAccess(Node (&nodes)[N], size_t size) noexcept :
                _nodes(nodes), _size(size)
            { }
        };

        class Iterator
        {
        public :
            Iterator() noexcept = default;
            Iterator(const Iterator&) noexcept = default;

            [[nodiscard]]
            friend inline bool operator==(
                const Iterator& lhs, const Iterator& rhs) noexcept
            {
                return lhs._nodes == rhs._nodes
                    && lhs._current == rhs._current;
            }

            [[nodiscard]]
            friend inline bool operator!=(
                const Iterator& lhs, const Iterator& rhs) noexcept
            {
                return !operator==(lhs, rhs);
            }

            Iterator& operator++()
            {
                assert(_nodes);
                assert(_current != -1);

                if (_nodes[_current].right != -1)
                {
                    goDeepOnLeft(_nodes[_current].right);
                }
                else
                {
                    goUp();
                }

                return *this;
            }

            [[nodiscard]]
            Iterator operator++(int)
            {
                assert(_nodes);
                assert(_current != -1);

                Iterator it = *this;

                if (_nodes[_current].right != -1)
                {
                    goDeepOnLeft(_nodes[_current].right);
                }
                else
                {
                    goUp();
                }

                return it;
            }

            [[nodiscard]]
            inline T& operator*() noexcept { return _nodes[_current].value; }

        private :
            friend class StaticBinaryTree;

            Node *_nodes = nullptr;
            int _current = -1;

            inline Iterator(Node *nodes, int current = -1) :
                _nodes(nodes)
            {
                goDeepOnLeft(current);
            }

            void goDeepOnLeft(int index)
            {
                assert(_nodes);

                if (index == -1)
                {
                    return;
                }

                while (_nodes[index].left != -1)
                {
                    index = _nodes[index].left;
                }

                _current = index;
            }

            void goUp()
            {
                assert(_nodes);
                assert(_current != -1);

                int prev = -1;

                do
                {
                    prev = _current;
                    _current = _nodes[_current].parent;
                }
                while (_current != -1 && _nodes[_current].left != prev);
            }
        };

        [[nodiscard]]
        static inline constexpr size_t maxSize() noexcept { return N; }

        StaticBinaryTree() noexcept(std::is_nothrow_default_constructible_v<T>)
        {
            initNextFreeNodes();
        }

        StaticBinaryTree(std::initializer_list<T> initList)
        {
            assert(initList.size() <= N);
            initNextFreeNodes();

            for (auto value : initList)
            {
                insert(std::move(value));
            }
        }

        [[nodiscard]]
        inline bool empty() const noexcept { return _size == 0; }

        [[nodiscard]]
        inline int size() const noexcept { return _size; }

        inline bool insert(T&& value)
        {
            assert(_size < N);

            bool inserted = false;

            _insert(std::forward<T>(value),
                    _nodes[0].parent,
                    _nodes[0].index,
                    inserted);

            return inserted;
        }

        [[nodiscard]]
        inline bool find(const T& value) const noexcept
        {
            return _find(value, _nodes[0].index);
        }

        inline bool erase(const T& value)
        {
            return _erase(value, _nodes[0].index);
        }

        [[nodiscard]]
        inline ContiguousMemoryAccess cma() noexcept
        {
            return ContiguousMemoryAccess(_nodes, _size);
        }

        [[nodiscard]]
        inline Iterator begin() noexcept
        {
            return Iterator(_nodes, (_size > 0) ? 0 : -1);
        }

        [[nodiscard]]
        inline Iterator end() noexcept
        {
            return Iterator(_nodes);
        }

    private :
        struct Node
        {
            int parent = -1;
            int index = -1;
            T value = {};
            int left = -1;
            int right = -1;
            int nextFree = -1;
        };

        Node _nodes[N] = {};
        int _freeHead = 0;
        size_t _size = 0;

        void initNextFreeNodes() noexcept
        {
            for (int n = 0; n + 1 < N; ++n)
            {
                _nodes[n].nextFree = n + 1;
            }
        }

        int _insert(T&& value, int parent, int index, bool& inserted)
        {
            if (index == -1)
            {
                int indexToUse = _freeHead;

                _nodes[indexToUse].parent = parent;
                _nodes[indexToUse].index = indexToUse;
                _nodes[indexToUse].value = std::forward<T>(value);
                _freeHead = std::exchange(_nodes[indexToUse].nextFree, -1);
                ++_size;
                inserted = true;

                return indexToUse;
            }
            else if (value < _nodes[index].value)
            {
                _nodes[index].left =
                    _insert(std::forward<T>(value),
                            index,
                            _nodes[index].left,
                            inserted);
            }
            else if (value > _nodes[index].value)
            {
                _nodes[index].right =
                    _insert(std::forward<T>(value),
                            index,
                            _nodes[index].right,
                            inserted);
            }

            return index;
        }

        [[nodiscard]]
        bool _find(const T& value, int index) const noexcept
        {
            if (index == -1)
            {
                return false;
            }
            else if (value < _nodes[index].value)
            {
                return _find(value, _nodes[index].left);
            }
            else if (value > _nodes[index].value)
            {
                return _find(value, _nodes[index].right);
            }
            else
            {
                return true;
            }
        }

        inline void removeNode(int index) noexcept
        {
            int parent = _nodes[index].parent;

            if (parent != -1)
            {
                int& childOfParent =
                    (_nodes[parent].left == index) ?
                    _nodes[parent].left : _nodes[parent].right;

                childOfParent = -1;
            }

            _nodes[index] = {};
            _nodes[index].nextFree = _freeHead;
            _freeHead = index;
            --_size;
        }

        void removeSubtree(int index)
        {
            if (index == -1)
            {
                return;
            }

            removeSubtree(_nodes[index].left);
            removeSubtree(_nodes[index].right);
            removeNode(index);
        }

        [[nodiscard]]
        bool _erase(const T& value, int index)
        {
            if (index == -1)
            {
                return false;
            }
            else if (value < _nodes[index].value)
            {
                return _erase(value, _nodes[index].left);
            }
            else if (value > _nodes[index].value)
            {
                return _erase(value, _nodes[index].right);
            }
            else
            {
                removeSubtree(index);

                return true;
            }
        }
    };
}

TEST(StaticBinaryTree, Test_1)
{
    StaticBinaryTree<int, 5> sbt;

    EXPECT_EQ(sbt.maxSize(), 5);
    EXPECT_TRUE(sbt.empty());
    EXPECT_EQ(sbt.size(), 0);

    EXPECT_TRUE(sbt.insert(12));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 1);
    EXPECT_TRUE(sbt.find(12));
    EXPECT_FALSE(sbt.insert(12));

    EXPECT_TRUE(sbt.insert(25));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_TRUE(sbt.find(25));
    EXPECT_FALSE(sbt.insert(25));

    EXPECT_TRUE(sbt.insert(8));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_TRUE(sbt.find(8));
    EXPECT_FALSE(sbt.insert(8));

    EXPECT_TRUE(sbt.insert(3));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_TRUE(sbt.find(3));
    EXPECT_FALSE(sbt.insert(3));

    EXPECT_TRUE(sbt.insert(18));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 5);
    EXPECT_TRUE(sbt.find(18));
    // EXPECT_FALSE(!sbt.insert(18));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 8);
        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 8);
        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(8));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_FALSE(sbt.find(8));
    EXPECT_FALSE(sbt.erase(8));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(12));
    EXPECT_TRUE(sbt.empty());
    EXPECT_EQ(sbt.size(), 0);
    EXPECT_FALSE(sbt.find(12));
    EXPECT_FALSE(sbt.erase(12));
}

TEST(StaticBinaryTree, Test_2)
{
    StaticBinaryTree<int, 5> sbt = {5, 13, 10, 9, 2};

    EXPECT_EQ(sbt.maxSize(), 5);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 5);
    EXPECT_TRUE(sbt.find(5));
    EXPECT_TRUE(sbt.find(13));
    EXPECT_TRUE(sbt.find(10));
    EXPECT_TRUE(sbt.find(9));
    EXPECT_TRUE(sbt.find(2));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(2));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_FALSE(sbt.find(2));
    EXPECT_FALSE(sbt.erase(2));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(13));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 1);
    EXPECT_FALSE(sbt.find(13));
    EXPECT_FALSE(sbt.erase(13));
    EXPECT_FALSE(sbt.find(10));
    EXPECT_FALSE(sbt.erase(10));
    EXPECT_FALSE(sbt.find(9));
    EXPECT_FALSE(sbt.erase(9));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.insert(2));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_TRUE(sbt.find(2));
    EXPECT_FALSE(sbt.insert(2));

    EXPECT_TRUE(sbt.insert(-17));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_TRUE(sbt.find(-17));
    EXPECT_FALSE(sbt.insert(-17));

    EXPECT_TRUE(sbt.insert(42));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_TRUE(sbt.find(42));
    EXPECT_FALSE(sbt.insert(42));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(2));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_FALSE(sbt.find(2));
    EXPECT_FALSE(sbt.erase(2));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.insert(84));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_TRUE(sbt.find(84));
    EXPECT_FALSE(sbt.insert(84));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 84);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(*begin++, 84);
        EXPECT_EQ(begin, end);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
