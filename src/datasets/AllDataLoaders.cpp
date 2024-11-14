#include "AllDataLoaders.h"
#include "ClassRegistry.h"
#include "logging.h"

namespace {
    auto dlog = logging::make_log("DataLoaderRegistry");
}

typedef ClassRegistry<DataLoader> OurRegistry;

static OurRegistry &registry() {
    static OurRegistry instance;
    return instance;
}

namespace StdDict {
    bool registerDataLoader(std::string name,
                              std::function<std::unique_ptr<DataLoader>()> f)
    {
        return registry().registerClass(name, f);
    }

    std::unique_ptr<DataLoader> getDataLoader(std::string name) {
        auto result = registry().makeClass(name);

        if(result == nullptr) { 
            SPDLOG_LOGGER_ERROR(dlog, "No DataLoader matching '{}' found!", name);
        }

        return result;
    }

    std::vector<std::string> listDataLoaders() {
        return registry().listClasses();
    }
}

