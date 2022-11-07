#include "src/mutex_test.h"

int main()
{ 
    constexpr size_t loopCount = 1000;
    benchmark<SharedMutex>(loopCount);
    benchmark<NormalMutex>(loopCount);
    return 0;
}
