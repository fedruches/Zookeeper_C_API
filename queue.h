#ifndef QUEUE_H
#define QUEUE_H
#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <charconv>
#include <list>
#include <fstream>

#include <zookeeper/zookeeper.h>

#include "sync_primitive.h"
#include "suffix_generator.hpp"

void watcher(zhandle_t *zkH, int type, int state, const char *path, void *watcherCtx);

class Queue : public SyncPrimitive
{
public:
    // Конструктор проверяет, существует ли корневой узел, и создаёт его, если тот отсутствует.
    Queue(const std::string &address, const std::string &name);

    // Add element to the queue
    bool produce(int i);

    int consume();

    void PrintProdDelimeter();
    void PrintConsDelimeter();

private:
    std::ofstream prodFile_;
    std::ofstream consFile_;
    std::string address_;
    std::string name_;
};

#endif // QUEUE_H
