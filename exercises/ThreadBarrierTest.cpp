#include <mutex>
#include <future>
#include <condition_variable>
#include <set>
#include <unordered_map>
#include <array>
#include <cassert>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define assertm(EXPR, MSG) assert((void(MSG), EXPR))

using testing::ElementsAre;
using testing::Pair;

using ThreadId_t = uint32_t;
using ThreadGroupId_t = uint32_t;

namespace
{
    // Thread barrier which release all blocked threads when waiting thread max is reached
    class ThreadBarrier
    {
    public :
        ThreadBarrier() = default;

        ThreadBarrier(size_t waitingThreadMax) : _waitingThreadMax(waitingThreadMax)
        {
            setWaitingThreadMax(waitingThreadMax);
        }

        void setWaitingThreadMax(size_t waitingThreadMax) noexcept
        {
            assertm(_waitingThreadMax > 0,
                    "waiting thread max must be higher than 0");
            _waitingThreadMax = waitingThreadMax;
        }

        void threadBarrierWait(ThreadId_t threadId, ThreadGroupId_t& threadGroupId)
        {
            std::unique_lock<std::mutex> lock(_mutex);

            if (_nbThreadWaiting == _waitingThreadMax - 1)
            {
                _nbThreadWaiting = 0;
                _condVar.notify_all();
                threadGroupId = _currentThreadGroupId;
                setNewThreadGroupId();
            }
            else
            {
                ++_nbThreadWaiting;
                _threadGroupIdAlreadyUsed.emplace(_currentThreadGroupId);
                _threadGroupIdByThreadId.emplace(threadId, _currentThreadGroupId);
                _condVar.wait(lock);
                threadGroupId = _threadGroupIdByThreadId.at(threadId);
                _threadGroupIdByThreadId.erase(threadId);
            }
        }

    private :
        size_t _waitingThreadMax = 1;
        std::mutex _mutex;
        std::condition_variable _condVar;
        uint32_t _nbThreadWaiting = 0;
        ThreadGroupId_t _currentThreadGroupId = 0;
        std::set<ThreadGroupId_t> _threadGroupIdAlreadyUsed;
        std::unordered_map<ThreadId_t, ThreadGroupId_t> _threadGroupIdByThreadId;

        inline void setNewThreadGroupId() noexcept
        {
            ThreadGroupId_t threadGroupId = 0;

            for (auto v : _threadGroupIdAlreadyUsed)
            {
                if (threadGroupId != v)
                {
                    break;
                }

                ++threadGroupId;
            }

            _currentThreadGroupId = threadGroupId;
        }
    };
}

TEST(ThreadBarrierTest, Test_1)
{
    using RetValue_t = std::pair<ThreadId_t, ThreadGroupId_t>;

    constexpr size_t THREADS_TOTAL = 10;
    ThreadBarrier threadBarrier(THREADS_TOTAL / 2);
    std::future<RetValue_t> results[THREADS_TOTAL];
    auto func = [&threadBarrier](ThreadId_t threadId) -> RetValue_t
    {
        ThreadGroupId_t threadGroupId;

        threadBarrier.threadBarrierWait(threadId, threadGroupId);

        return {threadId, threadGroupId};
    };

    for (uint32_t n = 0; n < THREADS_TOTAL; ++n)
    {
        results[n] = std::async(func, n);
    }

    std::array<RetValue_t, THREADS_TOTAL> retValues;

    for (uint32_t n = 0; n < retValues.size(); ++n)
    {
        retValues[n] = results[n].get();
    }

    EXPECT_THAT(retValues,
                ElementsAre(Pair(0, 0),
                            Pair(1, 0),
                            Pair(2, 0),
                            Pair(3, 0),
                            Pair(4, 0),
                            Pair(5, 1),
                            Pair(6, 1),
                            Pair(7, 1),
                            Pair(8, 1),
                            Pair(9, 1)));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
