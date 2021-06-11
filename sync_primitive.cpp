#include "sync_primitive.h"

std::mutex SyncPrimitive::lock_mutex;

SyncPrimitive::SyncPrimitive(std::string address)
{
    {
        if(zkHandler == nullptr)
        {
            try
            {
                std::cout << "Starting ZK:" << std::endl;
                zkHandler = zookeeper_init("localhost:2181", watcher, 10000, 0, 0, 0);

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
}

void SyncPrimitive::process(WatcherEvent event)
{
    std::lock_guard<std::mutex> lock(lock_mutex);
}
