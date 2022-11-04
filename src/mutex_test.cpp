#include "mutex_test.h"

int64_t SharedMutex::read() const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    _cv.wait(lock, [this] { return !_data.empty(); });
    std::this_thread::sleep_for(sleepUs);
    return _data.back(); 
}

void SharedMutex::write(int64_t data)
{
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        std::this_thread::sleep_for(sleepUs);
        _data.push(data);
    }
    _cv.notify_all();
}

void NormalMutex::write(int64_t data)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::this_thread::sleep_for(sleepUs);
        _data.push(data);
    }
    _cv.notify_all();
}

int64_t NormalMutex::read() const
{    
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return !_data.empty(); });
    std::this_thread::sleep_for(sleepUs);
    return _data.back();
}
