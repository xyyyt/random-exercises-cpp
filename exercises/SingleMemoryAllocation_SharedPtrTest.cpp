#include <cstdint>
#include <functional>
#include <gtest/gtest.h>

namespace
{
    constexpr struct InPlaceTag { } inPlaceTag;

    template <typename T>
    class SingleMemoryAllocation_SharedPtr
    {
        using ClassName_t = SingleMemoryAllocation_SharedPtr;

    public :
        SingleMemoryAllocation_SharedPtr() = default;

        SingleMemoryAllocation_SharedPtr(const ClassName_t& other)
        {
            copy(other);
        }

        SingleMemoryAllocation_SharedPtr(ClassName_t&& other)
        {
            move(other);
        }

        template <typename... Args>
        SingleMemoryAllocation_SharedPtr(InPlaceTag, Args&&... args)
        {
            allocate(std::forward<Args>(args)...);
        }

        ~SingleMemoryAllocation_SharedPtr() { destroy(); }

        SingleMemoryAllocation_SharedPtr& operator=(const ClassName_t& other)
        {
            if (this != &other)
            {
                copy(other);
            }

            return *this;
        }

        SingleMemoryAllocation_SharedPtr& operator=(ClassName_t&& other)
        {
            if (this != &other)
            {
                move(other);
            }

            return *this;
        }

        [[nodiscard]]
        operator bool() const noexcept { return _element != nullptr; }

        [[nodiscard]]
        T& operator*() const noexcept { return *_element; }

        [[nodiscard]]
        T *operator->() const noexcept { return _element; }

        [[nodiscard]]
        T *get() const noexcept { return _element; }

        [[nodiscard]]
        long useCount() const noexcept { return _refCounting ? *_refCounting : 0; }

        template <typename... Args>
        void reset(Args&&... args)
        {
            destroy();
            allocate(std::forward<Args>(args)...);
        }

        void reset(std::nullptr_t) { destroy(); }

    private :
        // memory block to encapsulate _element + _refCounting
        std::byte *_memoryBlock = nullptr;

        T *_element = nullptr;
        uint32_t *_refCounting = nullptr;

        void copy(const ClassName_t& other)
        {
            _memoryBlock = other._memoryBlock;
            _element = other._element;
            _refCounting = other._refCounting;

            if (_refCounting)
            {
                ++(*_refCounting);
            }
        }

        void move(ClassName_t& other)
        {
            destroy();

            _memoryBlock = other._memoryBlock;
            other._memoryBlock = nullptr;
            _element = other._element;
            other._element = nullptr;
            _refCounting = other._refCounting;
            other._refCounting = nullptr;
        }

        template <typename... Args>
        void allocate(Args&&... args)
        {
            constexpr uint32_t TOTAL_BLOCK_SIZE = sizeof(*_element) + sizeof(*_refCounting);

            _memoryBlock = reinterpret_cast<std::byte *>(operator new(TOTAL_BLOCK_SIZE));
            _element = reinterpret_cast<T *>(_memoryBlock);
            new (_element) T(std::forward<Args>(args)...);
            _refCounting = reinterpret_cast<uint32_t *>(_memoryBlock + sizeof(*_element));
            new (_refCounting) uint32_t(1);
        }

        void destroy()
        {
            if (_refCounting)
            {
                --(*_refCounting);

                if (!*_refCounting)
                {
                    _element->~T();
                    delete _memoryBlock;
                }

                _refCounting = nullptr;
                _element = nullptr;
                _memoryBlock = nullptr;
            }
        }
    };
}

TEST(SingleMemoryAllocation_SharedPtrTest, Test_1)
{
    SingleMemoryAllocation_SharedPtr<int> ptr;

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.operator->(), nullptr);
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.useCount(), 0);

    ptr.reset(42);

    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.operator*(), 42);
    EXPECT_EQ(*ptr.operator->(), 42);
    EXPECT_EQ(*ptr.get(), 42);
    EXPECT_EQ(ptr.useCount(), 1);

    auto ptr2 = ptr;

    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.operator*(), 42);
    EXPECT_EQ(*ptr2.operator->(), 42);
    EXPECT_EQ(*ptr2.get(), 42);
    EXPECT_EQ(ptr2.useCount(), 2);

    ptr.reset(nullptr);

    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr.operator->(), nullptr);
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.useCount(), 0);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.operator*(), 42);
    EXPECT_EQ(*ptr2.operator->(), 42);
    EXPECT_EQ(*ptr2.get(), 42);
    EXPECT_EQ(ptr2.useCount(), 1);

    auto ptr3 = std::move(ptr2);

    EXPECT_FALSE(ptr2);
    EXPECT_EQ(ptr2.operator->(), nullptr);
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr2.useCount(), 0);
    EXPECT_TRUE(ptr3);
    EXPECT_EQ(ptr3.operator*(), 42);
    EXPECT_EQ(*ptr3.operator->(), 42);
    EXPECT_EQ(*ptr3.get(), 42);
    EXPECT_EQ(ptr3.useCount(), 1);

    ptr3.reset(84);

    EXPECT_TRUE(ptr3);
    EXPECT_EQ(ptr3.operator*(), 84);
    EXPECT_EQ(*ptr3.operator->(), 84);
    EXPECT_EQ(*ptr3.get(), 84);
    EXPECT_EQ(ptr3.useCount(), 1);
}

TEST(SingleMemoryAllocation_SharedPtrTest, Test_2)
{
    SingleMemoryAllocation_SharedPtr<int> ptr;

    ptr.reset(nullptr);
    ptr.reset(21315);
    ptr.reset(nullptr);
    ptr.reset(345);
    ptr.reset(42);

    SingleMemoryAllocation_SharedPtr<int> ptr2;

    ptr2 = ptr;

    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.operator*(), 42);
    EXPECT_EQ(*ptr.operator->(), 42);
    EXPECT_EQ(*ptr.get(), 42);
    EXPECT_EQ(ptr.useCount(), 2);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.operator*(), 42);
    EXPECT_EQ(*ptr2.operator->(), 42);
    EXPECT_EQ(*ptr2.get(), 42);
    EXPECT_EQ(ptr2.useCount(), 2);

    ptr2 = std::move(ptr);

    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.operator*(), 42);
    EXPECT_EQ(*ptr2.operator->(), 42);
    EXPECT_EQ(*ptr2.get(), 42);
    EXPECT_EQ(ptr2.useCount(), 1);

    ptr.reset(nullptr);

    EXPECT_EQ(ptr.useCount(), 0);
    EXPECT_EQ(ptr2.useCount(), 1);

    ptr2.reset(nullptr);
    ptr2.reset(84);

    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.operator*(), 84);
    EXPECT_EQ(*ptr2.operator->(), 84);
    EXPECT_EQ(*ptr2.get(), 84);
    EXPECT_EQ(ptr2.useCount(), 1);

    SingleMemoryAllocation_SharedPtr<int> ptr3(inPlaceTag, *ptr2.get());

    EXPECT_EQ(ptr2.operator*(), 84);
    EXPECT_EQ(*ptr2.operator->(), 84);
    EXPECT_EQ(*ptr2.get(), 84);
    EXPECT_EQ(ptr2.useCount(), 1);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
