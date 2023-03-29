#include "client.h"

int main()
{
    DataBuffer buffer;

    CInput inputThread(buffer);
    std::thread t1(inputThread);

    CProccess proccessThread(buffer);
    std::thread t2(proccessThread);

    t1.join();
    t2.join();

    return 0;
}