#include <stdexcept>
#include <memory>
#include <gtest/gtest.h>

enum class CarModel : uint8_t { BMW, BUGATTI, PORSCHE };
enum class CarColor : uint8_t { BLACK, RED, WHITE };

class ICar
{
public :
    virtual ~ICar() = default;
    virtual CarModel getCarModel() const noexcept = 0;
    virtual CarColor getCarColor() const noexcept = 0;
    virtual uint32_t getNumberOfKmPerHour() const noexcept = 0;
    virtual bool isStrippable() const noexcept = 0;
};

class BMW : public ICar
{
public :
    BMW(CarColor color = CarColor::BLACK, bool isStrippable = false) noexcept :
        _color(color),
        _isStrippable(isStrippable)
    { }

    ~BMW() override = default;
    [[nodiscard]] CarModel getCarModel() const noexcept override { return CarModel::BMW; }
    [[nodiscard]] CarColor getCarColor() const noexcept override { return _color; }
    [[nodiscard]] uint32_t getNumberOfKmPerHour() const noexcept override { return 235; }
    [[nodiscard]] bool isStrippable() const noexcept override { return _isStrippable; };

private:
    CarColor _color;
    bool _isStrippable;
};

class Bugatti : public ICar
{
public :
    Bugatti(CarColor color = CarColor::BLACK, bool = false) noexcept : _color(color) { }
    ~Bugatti() override = default;
    [[nodiscard]] CarModel getCarModel() const noexcept override { return CarModel::BUGATTI; }
    [[nodiscard]] CarColor getCarColor() const noexcept override { return _color; }
    [[nodiscard]] uint32_t getNumberOfKmPerHour() const noexcept override { return 500; }
    [[nodiscard]] bool isStrippable() const noexcept override { return false; }

private :
    CarColor _color;
};

class Porsche : public ICar
{
public :
    Porsche(CarColor color = CarColor::BLACK, bool isStrippable = false) noexcept :
        _color(color),
        _isStrippable(isStrippable)
    { }

    ~Porsche() override = default;
    [[nodiscard]] CarModel getCarModel() const noexcept override { return CarModel::PORSCHE; }
    [[nodiscard]] CarColor getCarColor() const noexcept override { return _color; }
    [[nodiscard]] uint32_t getNumberOfKmPerHour() const noexcept override { return 294; }
    [[nodiscard]] bool isStrippable() const noexcept override { return _isStrippable; };

private:
    CarColor _color;
    bool _isStrippable = false;
};

class CarFactory final
{
public :
    CarFactory() = delete;
    ~CarFactory() = delete;

    template <CarModel CAR_MODEL, typename... Args>
    [[nodiscard]]
    static std::unique_ptr<ICar> createCar(Args&&... args)
    {
        switch (CAR_MODEL)
        {
           case CarModel::BMW :
               return std::make_unique<BMW>(std::forward<Args>(args)...);
           case CarModel::BUGATTI :
               return std::make_unique<Bugatti>(std::forward<Args>(args)...);
           case CarModel::PORSCHE :
               return std::make_unique<Porsche>(std::forward<Args>(args)...);
        }

        return nullptr;
    }
};

TEST(Factory, TestBMW)
{
    {
        auto bmw = CarFactory::createCar<CarModel::BMW>();

        EXPECT_EQ(bmw->getCarModel(), CarModel::BMW);
        EXPECT_EQ(bmw->getCarColor(), CarColor::BLACK);
        EXPECT_EQ(bmw->getNumberOfKmPerHour(), 235);
        EXPECT_FALSE(bmw->isStrippable());
    }

    {
        auto bmw = CarFactory::createCar<CarModel::BMW>(CarColor::RED);

        EXPECT_EQ(bmw->getCarModel(), CarModel::BMW);
        EXPECT_EQ(bmw->getCarColor(), CarColor::RED);
        EXPECT_EQ(bmw->getNumberOfKmPerHour(), 235);
        EXPECT_FALSE(bmw->isStrippable());
    }

    {
        auto bmw = CarFactory::createCar<CarModel::BMW>(CarColor::WHITE, true);

        EXPECT_EQ(bmw->getCarModel(), CarModel::BMW);
        EXPECT_EQ(bmw->getCarColor(), CarColor::WHITE);
        EXPECT_EQ(bmw->getNumberOfKmPerHour(), 235);
        EXPECT_TRUE(bmw->isStrippable());
    }
}

TEST(Factory, TestBugatti)
{
    {
        auto bugatti = CarFactory::createCar<CarModel::BUGATTI>();

        EXPECT_EQ(bugatti->getCarModel(), CarModel::BUGATTI);
        EXPECT_EQ(bugatti->getCarColor(), CarColor::BLACK);
        EXPECT_EQ(bugatti->getNumberOfKmPerHour(), 500);
        EXPECT_FALSE(bugatti->isStrippable());
    }

    {
        auto bugatti = CarFactory::createCar<CarModel::BUGATTI>(CarColor::RED);

        EXPECT_EQ(bugatti->getCarModel(), CarModel::BUGATTI);
        EXPECT_EQ(bugatti->getCarColor(), CarColor::RED);
        EXPECT_EQ(bugatti->getNumberOfKmPerHour(), 500);
        EXPECT_FALSE(bugatti->isStrippable());
    }

    {
        auto bugatti = CarFactory::createCar<CarModel::BUGATTI>(CarColor::WHITE);

        EXPECT_EQ(bugatti->getCarModel(), CarModel::BUGATTI);
        EXPECT_EQ(bugatti->getCarColor(), CarColor::WHITE);
        EXPECT_EQ(bugatti->getNumberOfKmPerHour(), 500);
        EXPECT_FALSE(bugatti->isStrippable());
    }
}

TEST(Factory, TestPorsche)
{
    {
        auto porsche = CarFactory::createCar<CarModel::PORSCHE>();

        EXPECT_EQ(porsche->getCarModel(), CarModel::PORSCHE);
        EXPECT_EQ(porsche->getCarColor(), CarColor::BLACK);
        EXPECT_EQ(porsche->getNumberOfKmPerHour(), 294);
        EXPECT_FALSE(porsche->isStrippable());
    }

    {
        auto porsche = CarFactory::createCar<CarModel::PORSCHE>(CarColor::RED);

        EXPECT_EQ(porsche->getCarModel(), CarModel::PORSCHE);
        EXPECT_EQ(porsche->getCarColor(), CarColor::RED);
        EXPECT_EQ(porsche->getNumberOfKmPerHour(), 294);
        EXPECT_FALSE(porsche->isStrippable());
    }

    {
        auto porsche = CarFactory::createCar<CarModel::PORSCHE>(CarColor::WHITE, true);

        EXPECT_EQ(porsche->getCarModel(), CarModel::PORSCHE);
        EXPECT_EQ(porsche->getCarColor(), CarColor::WHITE);
        EXPECT_EQ(porsche->getNumberOfKmPerHour(), 294);
        EXPECT_TRUE(porsche->isStrippable());
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
