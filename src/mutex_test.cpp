#include "mutex_test.h"

namespace {
void writeData(std::vector<int64_t>& dataBase, int64_t data)
{
    for (size_t i = 0; i < 2000; i++)
    {
        dataBase.push_back(i + data);
    }
}

int64_t findMax(const std::vector<int64_t>& dataBase)
{
    int64_t max = 0;
    for (const auto& data : dataBase)
    {
        if (max < data)
            max = data;
    }
    return max;
}
}

int64_t SharedMutex::read() const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    _cv.wait(lock, [this] { return !_dataBase.empty(); });
    auto max = findMax(_dataBase);
    return max;
}

void SharedMutex::write(int64_t data)
{
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        writeData(_dataBase, data);
    }
    _cv.notify_all();
}

void NormalMutex::write(int64_t data)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        writeData(_dataBase, data);
    }
    _cv.notify_all();
}

int64_t NormalMutex::read() const
{    
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return !_dataBase.empty(); });
    auto max = findMax(_dataBase);
    return max;
}
