#ifndef EVENTRECONSTRUCTOR_H
#define EVENTRECONSTRUCTOR_H

#include "datasets/include/DataBase.h"
#include "util/include/json.hpp"
#include "util/include/ClipBoard.h"

#include "util/include/util.hpp"
#include "util/include/logging.h"
#include "util/include/FEBookie.h"

namespace viz{
    class EventReconstructor{
        public:
            EventReconstructor() = delete;
            EventReconstructor(const uint8_t& total_fe);

            void configure(DataLoader* dataloader, uint8_t fe_id, const json& config = json());

            std::vector<std::vector<Event>> getEvents();
            std::vector<std::vector<Event>> synchronize(uint16_t index = 0); //Not in use right now

            void overwriteEvent(Event& arg_event, uint8_t fe_id);

            std::vector<std::shared_ptr<ClipBoard<EventData>>> clipboards;
            std::vector<std::vector<Event>> leftovers; //Storehouse for events that need to be reconstructed
            std::vector<uint32_t> refBCIDs; //For overriding bcid
            std::vector<uint32_t> refL1IDs; //For overriding l1id 


            uint8_t totalFEs, triggerMultiplier;
    };
}

#endif