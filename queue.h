#ifndef QUEUE_H
#define QUEUE_H
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <charconv>
#include <list>

#include <zookeeper/zookeeper.h>

#include "sync_primitive.h"

void watcher(zhandle_t *zkH, int type, int state, const char *path, void *watcherCtx);

class Queue : public SyncPrimitive
{
public:
    // Конструктор проверяет, существует ли корневой узел, и создаёт его, если тот отсутствует.
    Queue(const std::string &address, const std::string &name);

    // Add element to the queue
    bool produce(int i);

    int consume();

private:
    std::string address_;
    std::string name_;
};

#endif // QUEUE_H
