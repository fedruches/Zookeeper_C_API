#include "sync_primitive.h"
#include <memory>

std::mutex SyncPrimitive::lock_mutex;

SyncPrimitive::SyncPrimitive(std::string address)
{
    if(zkHandler == nullptr)
    {
        try
        {
            std::cout << "Starting ZK:" << std::endl;
            zkHandler = zookeeper_init(address.c_str(), watcher, 10000, 0, 0, 0);

            if (zkHandler == nullptr)
                std::cout << strerror(errno) << std::endl;

            std::cout << "Finished starting ZK: " << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << std::endl;
            zkHandler = nullptr;
        }
        catch (...)
        {
            std::cout << "Unknown exception" << std::endl;
        }
    }
}

void SyncPrimitive::process(WatcherEvent event)
{
    std::lock_guard<std::mutex> lock(lock_mutex);
}
