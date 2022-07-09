/**
 * @file main.cpp
 * @author Fabian Baumeister (fabianbaumeister.com)
 * @brief Validating https://www.codeproject.com/Articles/1227690/How-a-weak-ptr-Might-Prevent-Full-Memory-Cleanup-o
 * @date 2022-07-09
 * 
 */

#include <iostream>
#include <fstream>
#include <memory>

#include <unistd.h>  // _SC_PAGE_SIZE

void printMemoryUsage()
{
    // Slightly modified from:
    // https://gist.github.com/thirdwing/da4621eb163a886a03c5

    // the two fields we want
    unsigned long vsize{};
    long rss{};
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
            >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> vsize >> rss;
    }

    const auto page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;  // in case x86-64 is configured to use 2MB pages
    const auto vm_usage = vsize / 1024.0;
    const auto resident_set = rss * page_size_kb;

    std::cout << "VM: " << vm_usage << "; RSS: " << resident_set << std::endl;
}


static constexpr auto DATASIZE = 100 * 1024 * 1024; // Make this large enough to see some impact

class DataArray
{
private:
    std::array<char, DATASIZE> m_data;
};

class DataPtr
{
private:
    std::unique_ptr<char[]> m_data{std::make_unique<char[]>(DATASIZE)};
};


template<class DataType> void createAndDestroy_makeShared()
{
    std::cout << "----- MakeShared" << std::endl;
    auto shared = std::make_shared<DataType>();
    std::weak_ptr<DataType> weak = shared;

    std::cout << "Expectation: Increased Memory!" << std::endl;
    printMemoryUsage();

    shared.reset();

    // -> THIS EXPECTATION DOES NOT ALWAYS HOLD TRUE. SEE BELOW.
    std::cout << "Expectation: weak_ptr keeps ctrl-block alive and data allocated!" << std::endl;
    printMemoryUsage();

    weak.reset();  // verbose
    std::cout << "Expectation: All back to 'normal'" << std::endl;
    printMemoryUsage();
}

template<class DataType> void createAndDestroy_new()
{
    std::cout << "----- New" << std::endl;

    auto shared = std::shared_ptr<DataType>(new DataType);
    std::weak_ptr<DataType> weak = shared;

    std::cout << "Expectation: Increased Memory!" << std::endl;
    printMemoryUsage();

    shared.reset();

    std::cout << "Expectation: All back to 'normal'" << std::endl;
    printMemoryUsage();

    weak.reset();  // verbose
    std::cout << "Expectation: All back to 'normal'" << std::endl;
    printMemoryUsage();
}


int main()
{
    std::cout << "Memory at Start" << std::endl;
    printMemoryUsage();

    std::cout << "\n\nData inside Ctrl-Block" << std::endl;
    createAndDestroy_makeShared<DataArray>();
    createAndDestroy_new<DataArray>();

    /**
     * @brief The data block itself is stored on the heap by the unique_ptr.
     * The ctrl-block will not hold the data itself but will point to it.
     * The internal mechanics are the same, but the impact on memory consumption
     * are totally different.
     */
    std::cout << "\n\nData pointed to by Ctrl-Block" << std::endl;
    createAndDestroy_makeShared<DataPtr>();
    createAndDestroy_new<DataPtr>();

    std::cout << "\nMemory beofre Exit" << std::endl;
    printMemoryUsage();

    return 0;
}
