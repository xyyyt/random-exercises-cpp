#include <type_traits>
#include <thread>
#include <memory>
#include <gtest/gtest.h>

namespace
{
    template <typename T>
    class Singleton final
    {
    public :
        Singleton() = delete;
        ~Singleton() = delete;

        [[nodiscard]]
        static T& getInstance()
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_instance)
            {
                _instance = std::make_unique<T>();
            }

            return *_instance;
        }

    private :
        static std::mutex _mutex;
        static std::unique_ptr<T> _instance;
    };

    template <typename T>
    std::mutex Singleton<T>::_mutex;

    template <typename T>
    std::unique_ptr<T> Singleton<T>::_instance = nullptr;

    template <typename T>
    class MyFixture : public testing::Test
    {
    protected :
        using Type = T;
    };

    class NameGenerator
    {
    public :
        template <typename T>
        [[nodiscard]]
        static std::string GetName(int)
        {
            if constexpr (std::is_same_v<T, int>)
            {
                return "int";
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return "float";
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return "std::string";
            }
            else
            {
                testing::KilledBySignal(SIGABRT);
            }

            return "";
        }
    };
}

TYPED_TEST_SUITE_P(MyFixture);

TYPED_TEST_P(MyFixture, TestSingleton)
{
    using Type = typename std::decay_t<std::remove_pointer_t<decltype(this)>>::Type;

    auto& instance = Singleton<Type>::getInstance();

    if constexpr (!std::is_same_v<std::decay_t<decltype(instance)>, int>
                  && !std::is_same_v<std::decay_t<decltype(instance)>, float>
                  && !std::is_same_v<std::decay_t<decltype(instance)>, std::string>)
    {
        FAIL() << "Type not expected";
    }
}

REGISTER_TYPED_TEST_SUITE_P(MyFixture, TestSingleton);

using MyTypes = testing::Types<int, float, std::string>;

INSTANTIATE_TYPED_TEST_SUITE_P(MyInstantiation,
                               MyFixture,
                               MyTypes,
                               NameGenerator);

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
