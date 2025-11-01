#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <type_traits>
#include <functional>

using testing::AnyOf;
using testing::Not;
using testing::ElementsAre;

namespace
{
    template <typename T>
    class RecursiveSimpleLinkedList
    {
    public :
        RecursiveSimpleLinkedList() = default;
        RecursiveSimpleLinkedList(const RecursiveSimpleLinkedList& other) { copy(other); }
        ~RecursiveSimpleLinkedList() { clear(); }

        [[nodiscard]]
        bool operator==(const RecursiveSimpleLinkedList& other) const noexcept
        {
            return _size == other._size && _equal(_head, other._head);
        }

        [[nodiscard]]
        bool operator!=(const RecursiveSimpleLinkedList& other) const noexcept
        {
            return !operator==(other);
        }

        void copy(const RecursiveSimpleLinkedList& other)
        {
            clear();
            _size = other._size;
            _head = _copy(other._head);
        }

        [[nodiscard]]
        size_t size() const noexcept { return _size; }

        [[nodiscard]]
        bool empty() const noexcept{ return _size == 0; }

        void add(const T& value)
        {
            _head = _add(value, _head);
            ++_size;
        }

        [[nodiscard]]
        bool search(const T& value) const noexcept { return _search(value, _head); }

        bool erase(const T& value)
        {
            bool found = false;

            _head = _erase(value, _head, found);

            if (found)
            {
                --_size;
            }

            return found;
        }

        void clear()
        {
            _clear(_head);
            _head = nullptr;
            _size = 0;
        }

        void reverse() noexcept { _head = _reverse(_head, nullptr); }

        template <typename Functor>
        void apply(Functor functor) const noexcept { _apply(functor, _head); }

        template <typename Functor>
        void reversedApply(Functor functor) const noexcept { _reversedApply(functor, _head); }

    private :
        /* No tail node + raw pointer instead of std::shared_ptr<Node>
           to deliberately complicate the exercice */
        struct Node
        {
            T value;
            Node *next;
        };

        size_t _size = 0;
        Node *_head = nullptr;

        [[nodiscard]]
        bool _equal(const Node *first, const Node *second) const noexcept
        {
            if (!first && !second)
            {
                return true;
            }
            else if (!first && second || first && !second || first->value != second->value)
            {
                return false;
            }
            else
            {
                return _equal(first->next, second->next);
            }
        }

        [[nodiscard]]
        Node *_copy(const Node *other)
        {
            if (!other)
            {
                return nullptr;
            }

            Node *node = new Node;

            node->value = other->value;
            node->next = _copy(other->next);

            return node;
        }

        [[nodiscard]]
        Node *_add(const T& value, Node *current)
        {
            if (!current)
            {
                Node *node = new Node;

                node->value = value;
                node->next = nullptr;

                return node;
            }

            current->next = _add(value, current->next);

            return current;
        }

        [[nodiscard]]
        bool _search(const T& value, Node *current) const noexcept
        {
            if (!current)
            {
                return false;
            }
            else if (current->value == value)
            {
                return true;
            }
            else
            {
                return _search(value, current->next);
            }
        }

        [[nodiscard]]
        Node *_erase(const T& value, Node *current, bool& found)
        {
            if (!current)
            {
                return nullptr;
            }
            else if (current->value == value)
            {
                found = true;

                Node *next = current->next;

                delete current;

                return next;
            }
            else
            {
                current->next = _erase(value, current->next, found);

                return current;
            }
        }

        void _clear(Node *current)
        {
            if (!current)
            {
                return;
            }

            Node *next = current->next;

            delete current;

            _clear(next);
        }

        [[nodiscard]]
        Node *_reverse(Node *current, Node *prev) noexcept
        {
            if (!current)
            {
                return prev;
            }

            Node *head = _reverse(current->next, current);

            current->next = prev;

            return head;
        }

        template <typename Functor>
        void _apply(Functor functor, const Node *current) const noexcept
        {
            if (!current)
            {
                return;
            }

            functor(current->value);
            _apply(functor, current->next);
        }

        template <typename Functor>
        void _reversedApply(Functor functor, const Node *current) const noexcept
        {
            if (!current)
            {
                return;
            }

            _reversedApply(functor, current->next);
            functor(current->value);
        }
    };
}

#define TEST_ON_ALL_ELEMENTS(ELEM_TYPE, APPLY_FUNC_NAME, LIST, ...)     \
    do                                                                  \
    {                                                                   \
        std::vector<ELEM_TYPE> vectorToTest;                            \
                                                                        \
        vectorToTest.reserve(LIST.size());                              \
        LIST.APPLY_FUNC_NAME([&vectorToTest](ELEM_TYPE value)           \
        {                                                               \
            vectorToTest.emplace_back(value);                           \
        });                                                             \
                                                                        \
        EXPECT_THAT(vectorToTest, ElementsAre(__VA_ARGS__));            \
    }                                                                   \
    while (0)

TEST(RecursiveSimpleLinkedListTest, Test_1)
{
    RecursiveSimpleLinkedList<int> linkedList;

    EXPECT_TRUE(linkedList.empty());

    linkedList.add(42);
    linkedList.add(367);
    linkedList.add(5);
    linkedList.add(93);

    EXPECT_EQ(linkedList.size(), 4);
    EXPECT_TRUE(linkedList.search(42));
    EXPECT_TRUE(linkedList.search(5));
    EXPECT_TRUE(linkedList.search(93));
    EXPECT_FALSE(linkedList.search(-2334));
    EXPECT_FALSE(linkedList.search(4));
    EXPECT_FALSE(linkedList.search(-12));
    EXPECT_TRUE(linkedList.erase(367));
    EXPECT_TRUE(linkedList.erase(5));
    EXPECT_TRUE(linkedList.erase(42));
    EXPECT_FALSE(linkedList.erase(42));
    EXPECT_FALSE(linkedList.erase(-59));
    EXPECT_FALSE(linkedList.erase(0));
    EXPECT_EQ(linkedList.size(), 1);

    linkedList.clear();

    EXPECT_TRUE(linkedList.empty());
}

TEST(RecursiveSimpleLinkedListTest, Test_2)
{
    RecursiveSimpleLinkedList<int> linkedList;

    EXPECT_TRUE(linkedList.empty());

    linkedList.add(0);
    linkedList.erase(0);

    EXPECT_TRUE(linkedList.empty());

    linkedList.add(5);
    linkedList.add(12);
    linkedList.add(346);
    linkedList.add(5);

    EXPECT_EQ(linkedList.size(), 4);
    EXPECT_TRUE(linkedList.erase(5));
    EXPECT_TRUE(linkedList.erase(12));
    EXPECT_TRUE(linkedList.erase(346));
    EXPECT_TRUE(linkedList.erase(5));
    EXPECT_TRUE(linkedList.empty());

    RecursiveSimpleLinkedList<int> linkedList2 = linkedList;

    EXPECT_TRUE(linkedList2 == linkedList);
    EXPECT_TRUE(linkedList2.empty());

    linkedList2.add(-1);
    linkedList2.add(-54);
    linkedList2.add(412);

    EXPECT_EQ(linkedList2.size(), 3);
    TEST_ON_ALL_ELEMENTS(int, apply, linkedList2, -1, -54, 412);
    TEST_ON_ALL_ELEMENTS(int, reversedApply, linkedList2, 412, -54, -1);

    linkedList2.reverse();

    TEST_ON_ALL_ELEMENTS(int, apply, linkedList2, 412, -54, -1);
    TEST_ON_ALL_ELEMENTS(int, reversedApply, linkedList2, -1, -54, 412);

    EXPECT_FALSE(linkedList2.empty());
}

TEST(RecursiveSimpleLinkedListTest, Test_3)
{
    RecursiveSimpleLinkedList<int> linkedList;

    EXPECT_TRUE(linkedList.empty());
    EXPECT_FALSE(linkedList.search(1));
    EXPECT_FALSE(linkedList.search(2));
    EXPECT_FALSE(linkedList.search(3));
    EXPECT_FALSE(linkedList.search(4));
    EXPECT_FALSE(linkedList.search(5));

    linkedList.add(1);
    linkedList.add(2);
    linkedList.add(3);
    linkedList.add(4);
    linkedList.add(5);

    EXPECT_EQ(linkedList.size(), 5);
    EXPECT_TRUE(linkedList.search(1));
    EXPECT_TRUE(linkedList.search(2));
    EXPECT_TRUE(linkedList.search(3));
    EXPECT_TRUE(linkedList.search(4));
    EXPECT_TRUE(linkedList.search(5));

    RecursiveSimpleLinkedList<int> linkedList2;

    EXPECT_TRUE(linkedList2 != linkedList);
    EXPECT_TRUE(linkedList2.empty());

    linkedList2.add(3231);
    linkedList2.add(2764);

    RecursiveSimpleLinkedList<int> linkedList3 = linkedList2;

    EXPECT_TRUE(linkedList3 == linkedList2);
    EXPECT_EQ(linkedList3.size(), 2);
    EXPECT_TRUE(linkedList3.search(3231));
    EXPECT_TRUE(linkedList3.search(2764));

    linkedList2.copy(linkedList);

    EXPECT_EQ(linkedList2.size(), 5);
    TEST_ON_ALL_ELEMENTS(int, apply, linkedList2, 1, 2, 3, 4, 5);
    TEST_ON_ALL_ELEMENTS(int, reversedApply, linkedList2, 5, 4, 3, 2, 1);

    linkedList.copy(linkedList3);

    EXPECT_EQ(linkedList.size(), 2);
    TEST_ON_ALL_ELEMENTS(int, apply, linkedList, 3231, 2764);
    TEST_ON_ALL_ELEMENTS(int, reversedApply, linkedList, 2764, 3231);

    linkedList.reverse();

    TEST_ON_ALL_ELEMENTS(int, apply, linkedList, 2764, 3231);
    TEST_ON_ALL_ELEMENTS(int, reversedApply, linkedList, 3231, 2764);

    EXPECT_TRUE(linkedList3.erase(2764));

    EXPECT_EQ(linkedList3.size(), 1);
    EXPECT_FALSE(linkedList3.search(2764));

    EXPECT_TRUE(linkedList2.erase(5));
    EXPECT_TRUE(linkedList2.erase(1));
    EXPECT_TRUE(linkedList2.erase(3));

    EXPECT_EQ(linkedList2.size(), 2);

    linkedList.clear();

    EXPECT_TRUE(linkedList.empty());

    RecursiveSimpleLinkedList<int> linkedList4 = linkedList;

    linkedList4.copy(linkedList4);

    EXPECT_TRUE(linkedList4.empty());

    linkedList4.copy(linkedList2);

    EXPECT_TRUE(linkedList4 == linkedList2);
    EXPECT_TRUE(linkedList4.erase(4));
    EXPECT_TRUE(linkedList4.erase(2));
    EXPECT_TRUE(linkedList4.empty());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
