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

            void configure(const json& config);
            void connectDataLoader(DataLoader* dataloader, uint8_t fe_id);
            void connectDataLoaders(std::vector<std::unique_ptr<DataLoader>>& dataloaders);

            //unique_ptr to force user to move data rather than copy
            std::unique_ptr<FEEvents> getEvents();
            std::unique_ptr<FEEvents> synchronize(uint8_t index = 0); //Not in use right now

            size_t getLeftoversSize(uint8_t fe_id) const;
            inline uint8_t getTotalFEs() {return totalFEs;} 
        private:
            void overwriteEvent(Event& arg_event, uint8_t fe_id);

            std::vector<std::shared_ptr<ClipBoard<EventData>>> clipboards;
            std::vector<std::vector<Event>> leftovers; //Storehouse for events that need to be reconstructed
            std::vector<uint32_t> refBCIDs; //For overriding bcid
            std::vector<uint32_t> refL1IDs; //For overriding l1id 

            uint8_t totalFEs, triggerMultiplier;
    };

    
}

#endif