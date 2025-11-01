#include <utility>
#include <type_traits>
#include <cstddef>
#include <gtest/gtest.h>

namespace
{
    template <typename T, size_t N>
    class StaticLinkedList
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
                    assert(_index >= 0 && _index < N);

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
            friend class StaticLinkedList;

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
                _current = _nodes[_current].next;

                return *this;
            }

            [[nodiscard]]
            Iterator operator++(int)
            {
                assert(_nodes);
                assert(_current != -1);

                Iterator it = *this;

                _current = _nodes[_current].next;

                return it;
            }

            [[nodiscard]]
            inline T& operator*() noexcept { return _nodes[_current].value; }

        private :
            friend class StaticLinkedList;

            Node *_nodes = nullptr;
            int _current = -1;

            inline Iterator(Node *nodes, int current = -1) noexcept :
                _nodes(nodes), _current(current)
            { }
        };

        [[nodiscard]]
        static inline constexpr size_t maxSize() noexcept { return N; }

        StaticLinkedList() noexcept(std::is_nothrow_default_constructible_v<T>)
        {
            initNextFreeNodes();
        }

        StaticLinkedList(std::initializer_list<T> initList)
        {
            assert(initList.size() <= N);
            initNextFreeNodes();

            for (auto value : initList)
            {
                [[maybe_unused]] bool inserted = false;

                insert(std::move(value));
            }
        }

        [[nodiscard]]
        inline bool empty() const noexcept { return _size == 0; }

        [[nodiscard]]
        inline int size() const noexcept { return _size; }

        void insert(T&& value)
        {
            assert(_size < N);

            int index = _freeHead;

            _nodes[index].index = index;
            _nodes[index].value = std::forward<T>(value);
            _freeHead = std::exchange(_nodes[index].nextFree, -1);

            if (_head == -1)
            {
                _head = index;
            }

            if (_tail != -1)
            {
                _nodes[_tail].next = index;
            }

            _tail = index;
            ++_size;
        }

        [[nodiscard]]
        bool find(const T& value) const noexcept
        {
            if (_head == -1)
            {
                return false;
            }

            int current = _head;

            do
            {
                if (_nodes[current].value == value)
                {
                    return true;
                }

                current = _nodes[current].next;
            }
            while (current != -1);

            return false;
        }

        bool erase(const T& value) noexcept
        {
            if (_head == -1)
            {
                return false;
            }

            int prev = -1;
            int current = _head;

            do
            {
                if (_nodes[current].value == value)
                {
                    removeNode(prev, current);

                    return true;
                }

                prev = current;
                current = _nodes[current].next;
            }
            while (current != -1);

            return false;
        }

        [[nodiscard]]
        inline ContiguousMemoryAccess cma() noexcept
        {
            return ContiguousMemoryAccess(_nodes, _size);
        }

        [[nodiscard]]
        inline Iterator begin() noexcept
        {
            return Iterator(_nodes, _head);
        }

        [[nodiscard]]
        inline Iterator end() noexcept
        {
            return Iterator(_nodes);
        }

    private :
        struct Node
        {
            int index = -1;
            T value = {};
            int next = -1;
            int nextFree = -1;
        };

        Node _nodes[N] = {};
        int _freeHead = 0;
        int _head = -1;
        int _tail = -1;
        size_t _size = 0;

        void initNextFreeNodes() noexcept
        {
            for (int n = 0; n + 1 < N; ++n)
            {
                _nodes[n].nextFree = n + 1;
            }
        }

        inline void removeNode(int prev, int current) noexcept
        {
            if (_head == current && _tail == current)
            {
                _head = -1;
                _tail = -1;
            }
            else if (_head == current)
            {
                _head = _nodes[current].next;
            }
            else if (_tail == current)
            {
                _tail = prev;
            }

            if (prev != -1)
            {
                _nodes[prev].next = _nodes[current].next;
            }

            _nodes[current] = {};
            _nodes[current].nextFree = _freeHead;
            _freeHead = current;
            --_size;
        }
    };
}

TEST(StaticLinkedList, Test_1)
{
    StaticLinkedList<int, 5> sbt;

    EXPECT_EQ(sbt.maxSize(), 5);
    EXPECT_TRUE(sbt.empty());
    EXPECT_EQ(sbt.size(), 0);

    sbt.insert(12);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 1);
    EXPECT_TRUE(sbt.find(12));

    sbt.insert(25);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_TRUE(sbt.find(25));

    sbt.insert(8);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_TRUE(sbt.find(8));

    sbt.insert(3);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_TRUE(sbt.find(3));

    sbt.insert(18);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 5);
    EXPECT_TRUE(sbt.find(18));

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

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 8);
        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(8));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_FALSE(sbt.find(8));
    EXPECT_FALSE(sbt.erase(8));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 12);
        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(12));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_FALSE(sbt.find(12));
    EXPECT_FALSE(sbt.erase(12));

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 25);
        EXPECT_EQ(*begin++, 3);
        EXPECT_EQ(*begin++, 18);
        EXPECT_EQ(begin, end);
    }
}

TEST(StaticLinkedList, Test_2)
{
    StaticLinkedList<int, 5> sbt = {5, 13, 10, 9, 2};

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

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(*begin++, 2);
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
        EXPECT_EQ(*begin++, 13);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(13));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_FALSE(sbt.find(13));
    EXPECT_FALSE(sbt.erase(13));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 10);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(10));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_FALSE(sbt.find(10));
    EXPECT_FALSE(sbt.erase(10));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 9);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(9));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 1);
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

    sbt.insert(2);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 2);
    EXPECT_TRUE(sbt.find(2));

    sbt.insert(-17);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_TRUE(sbt.find(-17));

    sbt.insert(42);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_TRUE(sbt.find(42));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 2);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    EXPECT_TRUE(sbt.erase(2));
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 3);
    EXPECT_FALSE(sbt.find(2));
    EXPECT_FALSE(sbt.erase(2));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(begin, end);
    }

    sbt.insert(84);
    EXPECT_FALSE(sbt.empty());
    EXPECT_EQ(sbt.size(), 4);
    EXPECT_TRUE(sbt.find(84));

    {
        auto cma = sbt.cma();
        auto begin = cma.begin();
        auto end = cma.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, 42);
        EXPECT_EQ(*begin++, -17);
        EXPECT_EQ(*begin++, 84);
        EXPECT_EQ(begin, end);
    }

    {
        auto begin = sbt.begin();
        auto end = sbt.end();

        EXPECT_EQ(*begin++, 5);
        EXPECT_EQ(*begin++, -17);
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
