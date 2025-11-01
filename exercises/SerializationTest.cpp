#include <memory>
#include <map>
#include <array>
#include <type_traits>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define assertm(EXPR, MSG) assert((void(MSG), EXPR))

using testing::ElementsAre;
using testing::Pair;

namespace
{
    // SerDes = Serializer/Deserializer
    struct SerDes
    {
        struct SerializedData
        {
            using Memory = std::byte;

            size_t size;
            std::unique_ptr<Memory[]> data;
        };

        virtual ~SerDes() = default;
        virtual SerializedData serialize() const = 0;
        virtual void deserialize(const SerializedData&) = 0;
    };

    struct Struct_1 : SerDes
    {
        using SerDes::SerializedData;

        int n;
        float f;
        double d;

        Struct_1() = default;

        Struct_1(std::decay_t<decltype(n)> p_n,
                 std::decay_t<decltype(f)> p_f,
                 std::decay_t<decltype(d)> p_d) noexcept
            : n(p_n), f(p_f), d(p_d)
        { }

        ~Struct_1() override = default;

        [[nodiscard]]
        SerializedData serialize() const override
        {
            constexpr size_t TOTAL_FIELDS_SIZE = sizeof(n) + sizeof(f) + sizeof(d);
            SerializedData data =
            {
                TOTAL_FIELDS_SIZE,
                std::make_unique<SerializedData::Memory[]>(TOTAL_FIELDS_SIZE)
            };
            auto *ptr = data.data.get();

            *reinterpret_cast<std::decay_t<decltype(n)> *>(ptr) = n;
            ptr += sizeof(n);
            *reinterpret_cast<std::decay_t<decltype(f)> *>(ptr) = f;
            ptr += sizeof(f);
            *reinterpret_cast<std::decay_t<decltype(d)> *>(ptr) = d;
            ptr += sizeof(d);
            assertm(ptr - data.data.get() == TOTAL_FIELDS_SIZE,
                    "Some fields aren't be serialized yet");

            return data;
        }

        void deserialize(const SerializedData& data) override
        {
            assertm(data.size > 0 && data.data, "Serialized data is empty");

            const auto *ptr = data.data.get();

            n = *reinterpret_cast<const std::decay_t<decltype(n)> *>(ptr);
            ptr += sizeof(n);
            f = *reinterpret_cast<const std::decay_t<decltype(f)> *>(ptr);
            ptr += sizeof(f);
            d = *reinterpret_cast<const std::decay_t<decltype(d)> *>(ptr);
            ptr += sizeof(d);
            assertm(ptr - data.data.get() == data.size,
                    "Some fields aren't be deserialized yet");
        }
    };

    struct Struct_2 : SerDes
    {
        using SerDes::SerializedData;

        int n;
        char c;
        std::string s;

        Struct_2() = default;

        Struct_2(std::decay_t<decltype(n)> p_n,
                 std::decay_t<decltype(c)> p_c,
                 std::decay_t<decltype(s)> p_s) noexcept
            : n(p_n), c(p_c), s(std::move(p_s))
        { }

        ~Struct_2() override = default;

        [[nodiscard]]
        SerializedData serialize() const override
        {
            const size_t TOTAL_FIELDS_SIZE =
                sizeof(n) + sizeof(c) + sizeof(decltype(s.size())) + s.size();
            SerializedData data =
            {
                TOTAL_FIELDS_SIZE,
                std::make_unique<SerializedData::Memory[]>(TOTAL_FIELDS_SIZE)
            };
            auto *ptr = data.data.get();

            *reinterpret_cast<std::decay_t<decltype(n)> *>(ptr) = n;
            ptr += sizeof(n);
            *reinterpret_cast<std::decay_t<decltype(c)> *>(ptr) = c;
            ptr += sizeof(c);

            using StringSize_t = std::decay_t<decltype(s.size())>;

            *reinterpret_cast<StringSize_t *>(ptr) = s.size();
            ptr += sizeof(StringSize_t);

            for (auto c : s)
            {
                *reinterpret_cast<std::decay_t<decltype(c)> *>(ptr) = c;
                ptr += sizeof(c);
            }

            assertm(ptr - data.data.get() == data.size,
                    "Some fields aren't be serialized yet");

            return data;
        }

        void deserialize(const SerializedData& data) override
        {
            assertm(data.size > 0 && data.data, "Serialized data is empty");

            const auto *ptr = data.data.get();

            n = *reinterpret_cast<const std::decay_t<decltype(n)> *>(ptr);
            ptr += sizeof(n);
            c = *reinterpret_cast<const std::decay_t<decltype(c)> *>(ptr);
            ptr += sizeof(c);

            using StringSize_t = std::decay_t<decltype(s.size())>;

            StringSize_t len = *reinterpret_cast<const StringSize_t *>(ptr);

            ptr += sizeof(len);
            s.reserve(len);

            for (uint32_t n = 0; n < len; ++n)
            {
                using Value_t = std::decay_t<decltype(s)>::value_type;

                s.push_back(*reinterpret_cast<const Value_t *>(ptr));
                ptr += sizeof(Value_t);
            }

            assertm(ptr - data.data.get() == data.size,
                    "Some fields aren't deserialized yet");
        }
    };

    struct Struct_3 : SerDes
    {
        using SerDes::SerializedData;

        short s;
        std::map<int, std::string> m;

        Struct_3() = default;

        Struct_3(std::decay_t<decltype(s)> p_s,
                 std::decay_t<decltype(m)> p_m) noexcept
            : s(p_s), m(std::move(p_m))
        { }

        ~Struct_3() override = default;

        [[nodiscard]]
        SerializedData serialize() const override
        {
            size_t total_fields_size = sizeof(s) + sizeof(decltype(m.size()));

            for (const auto& [key, value] : m)
            {
                total_fields_size += sizeof(key);
                total_fields_size += sizeof(decltype(value.size()));
                total_fields_size += value.size();
            }

            SerializedData data
            {
                total_fields_size,
                std::make_unique<SerializedData::Memory[]>(total_fields_size)
            };
            auto *ptr = data.data.get();

            *reinterpret_cast<std::decay_t<decltype(s)> *>(ptr) = s;
            ptr += sizeof(s);

            using MapSize_t = std::decay_t<decltype(m.size())>;

            *reinterpret_cast<MapSize_t *>(ptr) = m.size();
            ptr += sizeof(MapSize_t);

            for (const auto& [key, value] : m)
            {
                using Key_t = std::decay_t<decltype(key)>;

                *reinterpret_cast<Key_t *>(ptr) = key;
                ptr += sizeof(Key_t);

                using StringSize_t = std::decay_t<decltype(value.size())>;

                *reinterpret_cast<StringSize_t *>(ptr) = value.size();
                ptr += sizeof(StringSize_t);

                for (auto c : value)
                {
                    *reinterpret_cast<std::decay_t<decltype(c)> *>(ptr) = c;
                    ptr += sizeof(c);
                }
            }

            assertm(ptr - data.data.get() == data.size,
                    "Some fields aren't be serialized yet");

            return data;
        }

        void deserialize(const SerializedData& data) override
        {
            assertm(data.size > 0 && data.data, "Serialized data is empty");

            const auto *ptr = data.data.get();

            s = *reinterpret_cast<const std::decay_t<decltype(s)> *>(ptr);
            ptr += sizeof(s);

            using MapSize_t = std::decay_t<decltype(m.size())>;

            MapSize_t size = *reinterpret_cast<const MapSize_t *>(ptr);

            ptr += sizeof(size);

            for (uint32_t n = 0; n < size; ++n)
            {
                using Map_t = std::decay_t<decltype(m)>;
                using Key_t = Map_t::key_type;

                Key_t key = *reinterpret_cast<const Key_t *>(ptr);

                ptr += sizeof(key);

                using Value_t = Map_t::mapped_type;
                using StringSize_t = Value_t::size_type;

                StringSize_t len = *reinterpret_cast<const StringSize_t *>(ptr);

                ptr += sizeof(len);

                Value_t value;

                value.reserve(len);

                for (uint32_t n = 0; n < len; ++n)
                {
                    using StringValue_t = std::decay_t<decltype(value)>::value_type;

                    value.push_back(*reinterpret_cast<const StringValue_t *>(ptr));
                    ptr += sizeof(StringValue_t);
                }

                m.emplace(key, std::move(value));
            }

            assertm(ptr - data.data.get() == data.size,
                    "Some fields aren't deserialized yet");
        }
    };
}

TEST(SerializationTest, TestStruct_1)
{
    using SerializedData = Struct_1::SerializedData;

    Struct_1 structToSerialize{42, 84.5, 245.2};
    SerializedData data = structToSerialize.serialize();
    Struct_1 structToDeserialize;

    structToDeserialize.deserialize(data);
    EXPECT_EQ(structToDeserialize.n, 42);
    EXPECT_EQ(structToDeserialize.f, 84.5);
    EXPECT_EQ(structToDeserialize.d, 245.2);
}

TEST(SerializationTest, TestStruct_2)
{
    using SerializedData = Struct_2::SerializedData;

    Struct_2 structToSerialize{42, 'a', "Hello World !"};
    SerializedData data = structToSerialize.serialize();
    Struct_2 structToDeserialize;

    structToDeserialize.deserialize(data);
    EXPECT_EQ(structToDeserialize.n, 42);
    EXPECT_EQ(structToDeserialize.c, 'a');
    EXPECT_EQ(structToDeserialize.s, "Hello World !");
}

TEST(SerializationTest, TestStruct_3)
{
    using SerializedData = Struct_3::SerializedData;

    Struct_3 structToSerialize{3, {{8, "abcd" }, {2, "efgh"}, {253, "AETOP"}}};
    SerializedData data = structToSerialize.serialize();
    Struct_3 structToDeserialize;

    structToDeserialize.deserialize(data);
    EXPECT_EQ(structToDeserialize.s, 3);
    EXPECT_THAT(structToDeserialize.m,
                ElementsAre(Pair(2, "efgh"), Pair(8, "abcd"), Pair(253, "AETOP")));
}

TEST(SerializationTest, TestAllStructsWithVirtualisation)
{
    using SerializedData = SerDes::SerializedData;

    std::array<std::unique_ptr<SerDes>, 3> serializers;

    serializers[0] = std::make_unique<Struct_1>(
        10, 21.0, 57.7);
    serializers[1] = std::make_unique<Struct_2>(
        411, 'W', "WaWaZa");
    serializers[2] = std::make_unique<Struct_3>(
        15, std::map<int, std::string>{{1000, "ASD"}, {478, "GgH"}});

    std::array<SerializedData, serializers.size()> datas;

    for (uint32_t n = 0; n < serializers.size(); ++n)
    {
        datas[n] = serializers[n]->serialize();
    }

    std::array<std::unique_ptr<SerDes>, datas.size()> deserializers;

    deserializers[0] = std::make_unique<Struct_1>();
    deserializers[1] = std::make_unique<Struct_2>();
    deserializers[2] = std::make_unique<Struct_3>();

    for (uint32_t n = 0; n < datas.size(); ++n)
    {
        deserializers[n]->deserialize(datas[n]);
    }

    const auto& deserializedStruct_1 = static_cast<Struct_1&>(*deserializers[0]);

    EXPECT_EQ(deserializedStruct_1.n, 10);
    EXPECT_EQ(deserializedStruct_1.f, 21.0);
    EXPECT_EQ(deserializedStruct_1.d, 57.7);

    const auto& deserializedStruct_2 = static_cast<Struct_2&>(*deserializers[1]);

    EXPECT_EQ(deserializedStruct_2.n, 411);
    EXPECT_EQ(deserializedStruct_2.c, 'W');
    EXPECT_EQ(deserializedStruct_2.s, "WaWaZa");

    const auto& deserializedStruct_3 = static_cast<Struct_3&>(*deserializers[2]);

    EXPECT_EQ(deserializedStruct_3.s, 15);
    EXPECT_THAT(deserializedStruct_3.m, ElementsAre(Pair(478, "GgH"), Pair(1000, "ASD")));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
