#include "queue.h"

int SuffixGenerator::i = -1;
int SuffixGenerator::j = 0;

Queue::Queue(const std::string &address, const std::string &name) :
    SyncPrimitive(address),
    address_{address}, name_{name}
{
    prodFile_.open("/home/fedor/zookeeper_queue/test_data/prod_file.txt");
    consFile_.open("/home/fedor/zookeeper_queue/test_data/cons_file.txt");

    if (zkHandler != nullptr)
    {
        try
        {
            //auto retStat = std::make_unique<Stat>();
            Stat *retStat = nullptr;

            //checks the existence of a node in zookeeper synchronously
            /*auto retExists = */zoo_exists(zkHandler, name_.c_str(), 0, retStat);
            /* ZOK operation completed successfully
            * ZNONODE the node does not exist.
            * ZNOAUTH the client does not have permission.
            * ZBADARGUMENTS - invalid input parameters
            * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
            * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
            * */

            if (retStat == nullptr)
            {
                std::vector<int> data = {0};
                std::vector<char> pathBuffer(100);

                //const ACL_vector *openAclUnsafe = ZOO_OPEN_ACL_UNSAFE;

                auto retCreate = zoo_create(zkHandler,
                                            name_.c_str(),
                                            reinterpret_cast<char*>(data.data()),
                                            1,
                                            &ZOO_OPEN_ACL_UNSAFE,
                                            ZOO_PERSISTENT,
                                            pathBuffer.data(),
                                            100);

                if (retCreate != ZOK)
                    std::cout << "zoo_create error" << std::endl;
            }
            std::cout << "Queue constructor" << std::endl;
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
    std::string elementName = name_ + "/element" + SuffixGenerator::GetNext();
    std::vector<char> pathBuffer(100);

    auto byteArray = std::to_string(SuffixGenerator::j);

    auto retCreate = zoo_create(zkHandler,
                                elementName.c_str(),
                                byteArray.c_str(),
                                byteArray.size(),
                                &ZOO_OPEN_ACL_UNSAFE,
                                ZOO_PERSISTENT,
                                pathBuffer.data(),
                                100);

    std::cout << "retCreate: " << retCreate << std::endl;

    prodFile_ << SuffixGenerator::j << std::endl;

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

        auto retGetChildren = zoo_get_children(zkHandler, name_.c_str(), 1, &strings);
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

            // Писатель добавляет 00, 01, 02, 03, 04, 05, ... 05, 95, 96, 97, 98, 99, -98, -97, -96, ..., -5, -4, -3, -2, -1, 0.
            // Писатель пишет в файл число, которое он добавляет
            // Читатель пишет в другой файл число, которое он получает
            // Файлы должны совпадать

            std::string minNode = strings.data[0];
            std::cout << minNode << std::endl;

            int min = std::stoi(minNode.substr(7));

            std::vector<std::string> tempStrVec(strings.count);
            for (int i = 0; i < strings.count; ++i)
                tempStrVec[i] = strings.data[i];

            // Найдем, есть ли в массиве потомков два элемента (99 и -99), при наличии которых происходит изменение порядка сортировки
            auto numberOfMaxVals = std::count_if(tempStrVec.begin(), tempStrVec.end(),
                                                 [](auto str){ return std::abs(std::stoi(str.substr(7))) == maxCountValue; });

            bool isMaxValPass = (numberOfMaxVals == 2);

            std::vector<std::string> positiveVector;
            auto minElemIt = positiveVector.begin();
            if (isMaxValPass)
            {
                std::copy_if(tempStrVec.begin(), tempStrVec.end(),
                             std::back_inserter(positiveVector),
                             [](auto str){ return std::stoi(str.substr(7)) > 0; });

                minElemIt = std::min_element(positiveVector.begin(), positiveVector.end(),
                                            [](auto str1, auto str2){ return std::stoi(str1.substr(7)) < std::stoi(str2.substr(7)); });

                std::cout << "JUMP" << std::endl;
            }
            else
            {
                for (int i = 0; i < strings.count; ++i)
                {
                    std::string tempNode = strings.data[i];
                    int tempValue = std::stoi(tempNode.substr(7));

                    if(tempValue < min)
                    {
                        min = tempValue;
                        minNode = tempNode;
                    }
                }
            }

            std::string rootPlusMinNode = name_ + "/" + (positiveVector.empty() ? minNode : *minElemIt);
            std::cout << "Temporary value: " + rootPlusMinNode << std::endl;

            // Проверить, что minNode является нулевым эдементом в векторе
            // Переход через максимальное значение счётчика

            std::vector<char> buffer(100);
            int bufferSize = 100;
            auto retGetNode = zoo_get(zkHandler, rootPlusMinNode.c_str(), 0, buffer.data(), &bufferSize, stat.get());

            if (retGetNode != ZOK)
            {
                std::cout << "zoo_get error" << std::endl;
                return -1;
            }

            std::cout << std::atoi(buffer.data()) << std::endl;

            int retDeleteNode = zoo_delete(zkHandler, rootPlusMinNode.c_str(), 0);
            if (retDeleteNode != ZOK)
            {
                std::cout << "zoo_delete error" << std::endl;
                return -1;
            }

            consFile_ << std::atoi(buffer.data()) << std::endl;

            return std::atoi(buffer.data());
        }
    }
}

void Queue::PrintProdDelimeter()
{
    prodFile_ << std::endl;
}

void Queue::PrintConsDelimeter()
{
    consFile_ << std::endl;
}