#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <type_traits>
#include <functional>
#include <variant>
#include <string>
#include <unordered_set>
#include <algorithm>

using testing::StrictMock;

namespace
{
    template <typename T>
    class Acceptor
    {
    public :
        Acceptor(const T& internal) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            _internal(internal) { }

        template <typename Visitor>
        void accept(const Visitor& visitor) const noexcept(noexcept(visitor(_internal)))
        {
            visitor(_internal);
        }

    private :
        T _internal;
    };

    template <typename T>
    class VisitorMock
    {
    public :
        MOCK_METHOD(void, visit, (const T&), (const, noexcept));
    };

    class MyFixture : public testing::TestWithParam<std::variant<int, float, std::string>> { };
}

TEST_P(MyFixture, TestWithVisitor_Lambda)
{
    std::visit([](const auto& value) -> void
    {
        using Type = std::decay_t<decltype(value)>;
        using MyVisitorMock = StrictMock<VisitorMock<Type>>;

        Acceptor<Type> acceptor(value);
        MyVisitorMock visitorMock;
        auto visitor = [&visitorMock](const Type& value) noexcept -> void
        {
            visitorMock.visit(value);
        };

        EXPECT_CALL(visitorMock, visit(value)).Times(1);
        acceptor.accept(visitor);
    }, GetParam());
}

TEST_P(MyFixture, TestWithVisitor_StdFunction)
{
    std::visit([](const auto& value) -> void
    {
        using Type = std::decay_t<decltype(value)>;
        using MyVisitorMock = StrictMock<VisitorMock<Type>>;

        Acceptor<Type> acceptor(value);
        MyVisitorMock visitorMock;
        std::function<void (const Type&)> visitor = std::bind(
            &MyVisitorMock::visit, &visitorMock, std::placeholders::_1);

        EXPECT_CALL(visitorMock, visit(value)).Times(1);
        acceptor.accept(visitor);
    }, GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    MyInstantiation,
    MyFixture,
    testing::Values(int(42), float(25.5), std::string("Hello World")),
    [](const testing::TestParamInfo<MyFixture::ParamType>& info) -> std::string
    {
        std::ostringstream oss;

        oss << "ValueIs_" << std::boolalpha;

        if (std::holds_alternative<int>(info.param))
        {
            oss << std::get<int>(info.param);
        }
        else if (std::holds_alternative<float>(info.param))
        {
            oss << std::get<float>(info.param);
        }
        else
        {
            oss << std::get<std::string>(info.param);
        }

        const std::unordered_set<char> TO_REPLACE({'.', ' '});
        constexpr char CHANGE_TO = '_';
        std::string testName = oss.str();

        std::transform(testName.begin(),
                       testName.end(),
                       testName.begin(),
                       [&TO_REPLACE, CHANGE_TO](char c) -> char
                       {
                           return TO_REPLACE.count(c) ? CHANGE_TO : c;
                       });

        return testName;
    });

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
