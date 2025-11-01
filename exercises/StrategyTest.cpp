#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <gtest/gtest.h>

namespace
{
    struct ProfitInfo
    {
        uint32_t startIdx;
        uint32_t endIdx;
        int profit;

        ProfitInfo(uint32_t startIdxx, uint32_t endIdxx, int profitt) noexcept :
            startIdx(startIdxx),
            endIdx(endIdxx),
            profit(profitt)
        { }
    };
}

template <>
struct std::less<ProfitInfo>
{
    [[nodiscard]]
    bool operator()(const ProfitInfo& a, const ProfitInfo& b) const noexcept
    {
        return a.profit < b.profit;
    }
};

namespace
{
    class IProfitStrategy
    {
    public :
        virtual ~IProfitStrategy() = default;
        virtual std::optional<ProfitInfo> execute(const std::vector<int>&)
            const noexcept = 0;
    };

    class BestProfit : public IProfitStrategy
    {
    public :
        ~BestProfit() override = default;

        [[nodiscard]]
        std::optional<ProfitInfo> execute(const std::vector<int>& profits)
            const noexcept override
        {
            if (profits.empty())
            {
                return { };
            }

            ProfitInfo bestProfitInfo{0, 0, profits[0]};

            for (uint32_t n = 0; n < profits.size(); ++n)
            {
                auto trySetBestProfitInfo = [&bestProfitInfo](
                     uint32_t startIdx, uint32_t endIdx, int currentProfit) noexcept -> void
                {
                    if (currentProfit > bestProfitInfo.profit)
                    {
                        bestProfitInfo.startIdx = startIdx;
                        bestProfitInfo.endIdx = endIdx;
                        bestProfitInfo.profit = currentProfit;
                    }
                };
                int currentProfit = profits[n];

                trySetBestProfitInfo(n, n, currentProfit);

                for (uint32_t n2 = n + 1; n2 < profits.size(); ++n2)
                {
                    currentProfit += profits[n2];
                    trySetBestProfitInfo(n, n2, currentProfit);
                }
            }

            return bestProfitInfo;
        }
    };

    class WorstProfit : public IProfitStrategy
    {
    public :
        ~WorstProfit() override = default;

        [[nodiscard]]
        std::optional<ProfitInfo> execute(const std::vector<int>& profits)
            const noexcept override
        {
            if (profits.empty())
            {
                return { };
            }

            ProfitInfo worstProfitInfo{0, 0, profits[0]};

            for (uint32_t n = 0; n < profits.size(); ++n)
            {
                auto trySetWorstProfitInfo = [&worstProfitInfo](
                     uint32_t startIdx, uint32_t endIdx, int currentProfit) noexcept -> void
                {
                    if (currentProfit < worstProfitInfo.profit)
                    {
                        worstProfitInfo.startIdx = startIdx;
                        worstProfitInfo.endIdx = endIdx;
                        worstProfitInfo.profit = currentProfit;
                    }
                };
                int currentProfit = profits[n];

                trySetWorstProfitInfo(n, n, currentProfit);

                for (uint32_t n2 = n + 1; n2 < profits.size(); ++n2)
                {
                    currentProfit += profits[n2];
                    trySetWorstProfitInfo(n, n2, currentProfit);
                }
            }

            return worstProfitInfo;
        }
    };

    class MedianProfit : public IProfitStrategy
    {
    public :
        ~MedianProfit() override = default;

        [[nodiscard]]
        std::optional<ProfitInfo> execute(const std::vector<int>& profits)
            const noexcept override
        {
            if (profits.empty())
            {
                return { };
            }

            std::multiset<ProfitInfo> profitsInfos;

            for (uint32_t n = 0; n < profits.size(); ++n)
            {
                int currentProfit = profits[n];

                profitsInfos.emplace(n, n, currentProfit);

                for (uint32_t n2 = n + 1; n2 < profits.size(); ++n2)
                {
                    currentProfit += profits[n2];
                    profitsInfos.emplace(n, n2, currentProfit);
                }
            }

            auto it = profitsInfos.cbegin();

            std::advance(it, profitsInfos.size() / 2);

            return *it;
        }
    };

    class ProfitContext
    {
    public :
        ProfitContext() = default;

        ProfitContext(std::unique_ptr<IProfitStrategy> profitStrategy) noexcept :
            _profitStrategy(std::move(profitStrategy))
        { }

        void setProfitStrategy(std::unique_ptr<IProfitStrategy> profitStrategy) noexcept
        {
            _profitStrategy = std::move(profitStrategy);
        }

        [[nodiscard]]
        std::optional<ProfitInfo> execute(const std::vector<int>& profits) const noexcept
        {
            return _profitStrategy->execute(profits);
        }

    private :
        std::unique_ptr<IProfitStrategy> _profitStrategy;
    };
}

TEST(Strategy, Test_1)
{
    ProfitContext profitContext;

    profitContext.setProfitStrategy(std::make_unique<BestProfit>());

    {
        auto profitInfo = profitContext.execute({ });

        EXPECT_FALSE(profitInfo);

        profitInfo = profitContext.execute({1, -4, 2, 5});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 2);
            EXPECT_EQ(profitInfo->endIdx, 3);
            EXPECT_EQ(profitInfo->profit, 7);
        }

        profitInfo = profitContext.execute({40, 4, -17, 12, 34});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 0);
            EXPECT_EQ(profitInfo->endIdx, 4);
            EXPECT_EQ(profitInfo->profit, 73);
        }
    }


    profitContext.setProfitStrategy(std::make_unique<WorstProfit>());

    {
        auto profitInfo = profitContext.execute({ });

        EXPECT_FALSE(profitInfo);

        profitInfo = profitContext.execute({-2, -4, 2, 5});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 0);
            EXPECT_EQ(profitInfo->endIdx, 1);
            EXPECT_EQ(profitInfo->profit, -6);
        }

        profitInfo = profitContext.execute({-1, 6, 53, 12, -3});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 4);
            EXPECT_EQ(profitInfo->endIdx, 4);
            EXPECT_EQ(profitInfo->profit, -3);
        }
    }


    profitContext.setProfitStrategy(std::make_unique<MedianProfit>());

    {
        auto profitInfo = profitContext.execute({ });

        EXPECT_FALSE(profitInfo);

        profitInfo = profitContext.execute({-2, -4, 2, 5});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 0);
            EXPECT_EQ(profitInfo->endIdx, 3);
            EXPECT_EQ(profitInfo->profit, 1);
        }

        profitInfo = profitContext.execute({-1, 6, 53, 12, -3});

        EXPECT_TRUE(profitInfo);

        if (profitInfo)
        {
            EXPECT_EQ(profitInfo->startIdx, 0);
            EXPECT_EQ(profitInfo->endIdx, 2);
            EXPECT_EQ(profitInfo->profit, 58);
        }
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
