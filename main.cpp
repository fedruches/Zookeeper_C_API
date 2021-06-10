#include <stdio.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/proto.h>
#include <errno.h>

#include <iostream>
#include <mutex>
#include <memory>
#include <vector>

// Keeping track of the connection state
static int connected = 0;
static int expired   = 0;

// *zkHandler handles the connection with Zookeeper
static zhandle_t *zkHandler;
static std::mutex lock_mutex;

std::string root;

// watcher function would process events
void watcher(zhandle_t *zkH, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        // state refers to states of zookeeper connection.
        // To keep it simple, we would demonstrate these 3: ZOO_EXPIRED_SESSION_STATE, ZOO_CONNECTED_STATE, ZOO_NOTCONNECTED_STATE
        // If you are using ACL, you should be aware of an authentication failure state - ZOO_AUTH_FAILED_STATE
        if (state == ZOO_CONNECTED_STATE)
        {
            connected = 1;
        }
        else if (state == ZOO_NOTCONNECTED_STATE )
        {
            connected = 0;
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            expired = 1;
            connected = 0;
            zookeeper_close(zkH);
        }
    }
}

void SyncPrimitive(std::string address) {
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


void process(WatcherEvent event)
{
    std::lock_guard<std::mutex> lock(lock_mutex);
}

class Queue
{
    Queue(const std::string &address, const std::string &name) :
        address_{address}, name_{name}
    {
        if (zkHandler != nullptr)
        {
            try
            {
                auto retStat = std::make_unique<Stat>();

                //checks the existence of a node in zookeeper synchronously
                auto retExists = zoo_exists(zkHandler, name.c_str(), 0, retStat.get());
                /* ZOK operation completed successfully
                * ZNONODE the node does not exist.
                * ZNOAUTH the client does not have permission.
                * ZBADARGUMENTS - invalid input parameters
                * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
                * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
                * */
                if (retExists == ZNONODE)
                {
                    std::vector<char> data = {0};
                    std::vector<char> pathBuffer(100);

                    //const ACL_vector *openAclUnsafe = ZOO_OPEN_ACL_UNSAFE;

                    /*auto retCreate = */zoo_create(zkHandler,
                               name.c_str(),
                               data.data(),
                               1,
                               &ZOO_OPEN_ACL_UNSAFE,
                               ZOO_PERSISTENT,
                               pathBuffer.data(),
                               100);
                }
            }
            catch (const std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
            catch (...)
            {
                std::cout << "Unknown exception" << std::endl;
            }


        }


    }


private:
    std::string address_;
    std::string name_;
};


int main(){
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);

    // zookeeper_init returns the handler upon a successful connection, null otherwise
    zkHandler = zookeeper_init("localhost:2181", watcher, 10000, 0, 0, 0);

    if (!zkHandler) {
        std::cout << "Connection refused!" << std::endl;
        return errno;
    }else{
        printf("Connection established with Zookeeper. \n");
    }

    // Close Zookeeper connection
    zookeeper_close(zkHandler);

    return 0;
}