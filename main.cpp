#include <stdio.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/proto.h>
#include <errno.h>

#include <iostream>
#include <mutex>
#include <memory>
#include <vector>

#include "sync_primitive.h"
#include "queue.h"

// Keeping track of the connection state
static int connected = 0;
static int expired   = 0;

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
    std::cout << "Watcher" << std::endl;
}

// java SyncPrimitive qTest localhost 100 c
void QueueTest()
{
    Queue q("localhost:2181", "/app1");
    int max = 2;

    // produce
    for (int i = 0; i < max; ++i)
    {
        try
        {
            q.produce(i + 10);
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "Unkonwn exception" << std::endl;
        }
    }

    // consume
    for (int i = 0; i < max; ++i)
    {
        try
        {
            int c = q.consume();
            std::cout << "Item: " << c << std::endl;
        }
        catch (const std::exception &e)
        {
            i--;
            std::cout << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "Unkonwn exception" << std::endl;
        }
    }
}

int main()
{
    QueueTest();
    return 0;
}