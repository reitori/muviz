#include "FEBookie.h"

namespace viz{
    std::unordered_map<uint8_t, std::string> FEBookie::internal_map;

    void FEBookie::addFE(uint8_t fe_id, std::string name){
        internal_map.insert(std::make_pair(fe_id, name));
    }

    std::string FEBookie::getName(uint8_t fe_id){
        if(internal_map.find(fe_id) != internal_map.end())
            return internal_map.find(fe_id)->second;

        return "";
    }
}