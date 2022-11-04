#pragma once

#include <chrono>
#include <atomic>
#include <iostream>
#include <shared_mutex>
#include <queue>
#include <condition_variable>
#include <thread>

static const auto sleepUs = std::chrono::microseconds(0);

class SharedMutex {
public:
    int64_t read() const;
    void write(int64_t data);
    
private:
    mutable std::shared_mutex _mutex;
    mutable std::condition_variable_any _cv;
    std::queue<int64_t> _data;
};

class NormalMutex {
public:
    int64_t read() const;
    void write(int64_t data);

private:
    mutable std::mutex _mutex;
    mutable std::condition_variable _cv;
    std::queue<int64_t> _data;
};

enum class Operation { Read, Write };

template<typename MutexT>
class MutexBenchmark {
public:
    using Clock = std::chrono::high_resolution_clock;

    MutexBenchmark(size_t loopCount) : _loopCount(loopCount) {}

    void printTimeCost() 
    {
        std::cout << "Mutex type:" << typeid(MutexT).name() << " Loop count: " << _loopCount
        << " Reading time cost(us): " << _readerTimeCostUs.load() << " Writing time cost(us): " 
        << _writerTimeCostUs.load() << " sleepUs " << sleepUs.count() << std::endl;
    }

    void run(Operation op) 
    {
        const auto startTime = Clock::now();
        constexpr int64_t data = 100;

        for (size_t i = 0; i < _loopCount; ++i) {
            if (op == Operation::Read) {
                _mutex.read();
            } else {
                _mutex.write(data);
            }
        }

        const auto timeCostUs = 
            std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - startTime).count();
        if (op == Operation::Read) {
            _readerTimeCostUs += timeCostUs;
        }  else {
            _writerTimeCostUs += timeCostUs;
        }
    }

private:
    const size_t _loopCount;
    MutexT _mutex;
    std::atomic_int64_t _readerTimeCostUs{};
    std::atomic_int64_t _writerTimeCostUs{};
};

template<typename MutexT>
void benchmark(const size_t loopCount)
{
    MutexBenchmark<MutexT> bm(loopCount);
    {
        constexpr int readerCount = 4;
        constexpr int writerCount = 1;
        std::vector<std::jthread> readers;
        readers.reserve(readerCount);
        std::vector<std::jthread> writers;
        writers.reserve(writerCount);

        for (int i = 0; i < readerCount; i++) {
            readers.emplace_back([&] { bm.run(Operation::Read); });
        }
        for (int i = 0; i < writerCount; i++) {
            writers.emplace_back([&] { bm.run(Operation::Write); });
        }
    }
    bm.printTimeCost();
}
