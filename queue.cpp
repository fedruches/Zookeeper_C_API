#include "queue.h"

Queue::Queue(const std::string &address, const std::string &name) :
    SyncPrimitive(address),
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
                std::vector<int> data = {0};
                std::vector<char> pathBuffer(100);

                //const ACL_vector *openAclUnsafe = ZOO_OPEN_ACL_UNSAFE;

                /*auto retCreate = */zoo_create(zkHandler,
                                                name_.c_str(),
                                                reinterpret_cast<char*>(data.data()),
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

bool Queue::produce(int i)
{
    std::array<int, 1> value = {i};
    std::string elementName = name_ + "/element";
    std::vector<char> pathBuffer(100);

    auto retCreate = zoo_create(zkHandler,
                                elementName.c_str(),
                                reinterpret_cast<char*>(value.data()),
                                1,
                                &ZOO_OPEN_ACL_UNSAFE,
                                ZOO_PERSISTENT,
                                pathBuffer.data(),
                                100);

    if (retCreate != ZOK)
        return false;

    return true;
}

int Queue::consume()
{
    int retValue = -1;
    auto stat = std::make_unique<Stat>();

    while(true)
    {
        std::lock_guard<std::mutex> lock(lock_mutex);
        String_vector strings;

        auto retGetChildren = zoo_get_children(zkHandler, root.c_str(), 1, &strings);
        if (retGetChildren != ZOK)
        {
            std::cout << "zoo_get_children error" << std::endl;
            return -1;
        }

        if (strings.count == 0)
        {
            std::cout << "Going to wait" << std::endl;
        }
        else
        {
            // Здесь такая идея: функция get_сhildren() возвращает список детей в лексикографическом порядке.
            // Поскольку лексикографический порядок не обязательно следует числовому порядку значений счетчика,
            // нужно решить, какой элемент является наименьшим.
            // Для этого осуществляется проход по списку и из него удаляется префикс "element" для каждого узла.

            std::string minNode = strings.data[0];
            int min = std::stoi(minNode.substr(7));

            for (int i = 0; i < strings.count; ++i)
            {
                std::string tempNode = strings.data[i];
                int tempValue = std::stoi(tempNode.substr(7));
                std::cout << "Temporary value: " + std::to_string(tempValue)<< std::endl;
                if(tempValue < min)
                {
                    min = tempValue;
                    minNode = tempNode;
                }
            }
            std::cout << "Temporary value: " + root + "/" + minNode << std::endl;
            std::string rootPlusMinNode = root + "/" + minNode;

            std::vector<char> pathBuffer(100);
            int bufferSize = 100;
            auto retGetNode = zoo_get(zkHandler, rootPlusMinNode.c_str(), 0, pathBuffer.data(), &bufferSize, stat.get());

            if (retGetNode != ZOK)
            {
                std::cout << "zoo_get error" << std::endl;
                return -1;
            }

            int retDeleteNode = zoo_delete(zkHandler, rootPlusMinNode.c_str(), 0);
            if (retDeleteNode != ZOK)
            {
                std::cout << "zoo_delete error" << std::endl;
                return -1;
            }

            return stat->czxid;
        }
    }
}