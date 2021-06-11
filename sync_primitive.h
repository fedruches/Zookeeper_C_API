#ifndef SYNCPRIMITIVE_H
#define SYNCPRIMITIVE_H
#pragma once

#include <iostream>
#include <mutex>
#include <zookeeper/zookeeper.h>

// *zkHandler handles the connection with Zookeeper
static zhandle_t *zkHandler;

void watcher(zhandle_t *zkH, int type, int state, const char *path, void *watcherCtx);

// Конструктор этого класса проверяет, существует ли обработчик для ZooKeeper, и,
// если нет, пытается его создать
class SyncPrimitive
{
public:
    SyncPrimitive(std::string address);

    void process(WatcherEvent event);

private:
    std::string root;
    static std::mutex lock_mutex;
};

#endif // SYNCPRIMITIVE_H