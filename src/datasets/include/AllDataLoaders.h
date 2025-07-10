#ifndef ALL_DATA_LOADERS_H
#define ALL_DATA_LOADERS_H

#include "DataBase.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

using namespace viz;

namespace StdDict {
    bool registerDataLoader(std::string name,
                              std::function<std::unique_ptr<DataLoader>()> f);
    std::unique_ptr<DataLoader> getDataLoader(std::string name);

    std::vector<std::string> listDataLoaders();
}

#endif
