#include "queue.h"

Queue::Queue(const std::string &address, const std::string &name) :
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