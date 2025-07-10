#ifndef PARTICLE_EVENT_H
#define PARTICLE_EVENT_H

#include "Event.h"

namespace viz{
    namespace system{
        class ParticleHit : public event {
            public:
                ParticleHit(std::string name, unsigned int hits, unsigned int row, unsigned int col){
                    data = new eventData;
                    data->stringUIntPairedData = std::make_pair(name, hits);
                    data->uintPairedData = std::make_pair(row, col);
                }

                static eventType GetStaticType() { return eventType::particleHit; }
                eventType getEventType() const override { return eventType::particleHit; }
                eventCategory getEventCat() const override { return eventCategory::eventCatParticle; }
                eventData* getData() const override { return data; }

                std::string toString() const override {
                    std::stringstream ss;
                    ss << data->stringUIntPairedData.first << " Hit: at (" << data->uintPairedData.first << ", " << data->uintPairedData.second << ") with " << data->stringUIntPairedData.second << " total hits";
                    return ss.str();
                }
            private:
                eventData* data;
        };
    }
}


#endif