#include "OpenGL/Scene/Detector.h"

namespace {
    auto logger = logging::make_log("Detector");
}

//TODO: Move matrix computations to GPU. Main bottleneck right now is the amount of CPU computation for particle rendering.

namespace viz{
    Detector::Detector() : eventBatchTimestamps(10){
        ParticlesContainer.resize(totalParticles);
        m_cli; 
    }

    void Detector::init(const std::shared_ptr<VisualizerCli>& cli){
        m_cli = cli;

        logger->info("Initializing detector");

        auto masterConf = cli->getMasterConfig();
        if(masterConf.contains("viz_config")){
            //This parameter "total_detector_eventblocks" determines the size of
            //the CircularBuffer -- eventBatchTimestamps
            //Currently timing information is recorded inside of the detector update
            //when receiving event batches from the CorryWrapper. This is okay for cosmics
            //where the rate is low and when we don't want to worry about precise timing information
            //But at high-rate we might want finer timing information. This would require us to change
            //some code in the EventLoaderYarr module to get timing information down
            if(masterConf["viz_config"].contains("total_detector_eventblocks") && (masterConf["viz_config"])["total_detector_eventblocks"].is_number_unsigned()){
                unsigned int totalMemorySize = (masterConf["viz_config"])["total_detector_eventblocks"];
                eventBatchTimestamps = CircularBuffer<std::pair<EventBatch, std::chrono::steady_clock::time_point>>(totalMemorySize);
            }
        }

        for(int i = 0; i < cli->getTotalFEs(); i++) {
            const json& config = cli->getConfig(i);
            std::string name = config["name"].get<std::string>();
            std::vector<float> pos;
            if(config.contains("position")){
                pos = config["position"].get<std::vector<float>>();
            }else{
                pos = {0.0f, 0.0f, 0.0f};
                logger->warn("Position not given, assuming default value (0, 0, 0) for chip {}", name);
            }

            std::vector<float> angle;
            if(config.contains("angle")){
                angle = config["angle"].get<std::vector<float>>();
            }else{
                angle = {0.0f, 0.0f, 0.0f};
                logger->warn("Angle not given, assuming default value (0, 0, 0) for chip {}", name);
            }

            std::vector<float> size;
            if(config.contains("size")){
                size = config["size"].get<std::vector<float>>();
            }else{
                size = {1.0f, 1.0f, 0.1f};
                logger->warn("Size not given, assuming default value (1, 1, 0.1) for chip {}", name);
            }

            std::vector<float> rowcol;
            if(config.contains("rowcol")) {
                rowcol = config["rowcol"].get<std::vector<float>>();
            }else{
                logger->error("Please provide 'rowcol' for chip {}", name);
                throw std::runtime_error("Missing required 'rowcol' parameter for chip: " + name);
            }

            OrientationMode orientation;
            if(config.contains("orientation_mode")){
                orientation = config["orientation_mode"];
            }else{
                if(config.contains("corryvreckan_geometry_file"))
                    orientation = OrientationMode::XYZ;
                else{
                    orientation = OrientationMode::QUAT;
                }
            }

            bool usingRad;
            if(config.contains("angle_unit") && (config["angle_unit"] == std::string("rad") || config["angle_unit"] == std::string("radian"))){
                usingRad = true;
            }else{
                usingRad = false; // default to degrees
            }

            m_chips.emplace_back(true, rowcol[1], rowcol[0]);
            auto& currChip = m_chips.back();

            currChip.name = name;
            currChip.fe_id = i;
            currChip.pos = glm::vec3(pos[0], pos[1], pos[2]);
            currChip.eulerRot = glm::vec3(angle[0], angle[1], angle[2]);
            currChip.scale = glm::vec3(size[0] / 2.0f, size[1] / 2.0f, size[2] / 2.0f);
            currChip.hits = 0;
            currChip.orientation = orientation;

            glm::mat4 tempTfm = transform(currChip.scale, currChip.eulerRot, currChip.pos, orientation, usingRad);

            Particle tempPart;
            tempPart.pos = currChip.pos;
            tempPart.transform = tempTfm;
            tempPart.color = glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.65f);
            tempPart.is_immortal = true;
            tempPart.lifetime = 100.0f;
            tempPart.ndcDepth = 0.0f;
            ParticlesContainer[findUnusedParticle()] = tempPart;

        };

        //Find length of detector
        detectorLength = 0;
        for(int i = 0; i < m_chips.size(); i++){
            glm::vec3 thisPosition = m_chips[i].pos;
            for(int j = i + 1; j < m_chips.size(); j++){
                float testLength = (m_chips[j].pos - thisPosition).length();
                if(testLength > detectorLength){
                    detectorLength = testLength;
                }
            }
        }
        
        logger->info("Detector initialized with {0} chips", m_chips.size());
    }

    void Detector::setPlayback(std::chrono::steady_clock::time_point timePoint){
        useEventTimeStorageHead = false;
        const size_t sz = eventBatchTimestamps.getSize();

        if (sz > 2 && timePoint > eventBatchTimestamps[0].second && timePoint < eventBatchTimestamps[sz - 1].second) {
            timeSincePlaybackReset = timePoint;

            size_t chosenIndex = 0;
            for (size_t i = 0; i < sz; i++) {
                if (eventBatchTimestamps[i].second > timeSincePlaybackReset) {
                    chosenIndex = (i == 0) ? 0 : (i - 1);
                    break;
                }
            }

            if (chosenIndex >= sz) chosenIndex = sz - 1;
            lastEventBatchTimestampIndex = chosenIndex;
        } else {
            useEventTimeStorageHead = true;
            lastEventBatchTimestampIndex = (sz > 0) ? sz - 1 : 0;
        }
    }

    void Detector::update(const Camera& cam, const float& dTime){
        updateParticles(cam, dTime);
        std::chrono::duration<double> chronoDelTime(dTime);
        timeSincePlaybackReset += std::chrono::duration_cast<std::chrono::microseconds>(chronoDelTime);

        if(!m_cli)
            return;

        //First receive data and record timing
        auto newData = m_cli->getEventBatch();
        auto start = std::chrono::steady_clock::now();

        if(newData.first) //No hits or tracks in this case. //Store the batch data and timing in eventTimeStorage for playback purposes
            eventBatchTimestamps.push(std::move(std::make_pair(std::move(newData), start)));

        bool indexChanged = false;
        size_t ebtSize = eventBatchTimestamps.getSize();
        size_t playbackIndex = 0;
        if(ebtSize == 0){
            if(!newData.first) return;
        }else{
            playbackIndex = ebtSize - 1;
        }

        if(useEventTimeStorageHead){
            if(newData.first){ //When we are at the head, render data only when new data comes in
                indexChanged = true;
                std::cout << "should have rendered" << std::endl;
            }
        }
        else{
            if(lastEventBatchTimestampIndex + 1 < ebtSize){
                if(timeSincePlaybackReset >= eventBatchTimestamps[lastEventBatchTimestampIndex + 1].second){
                    playbackIndex = ++lastEventBatchTimestampIndex;
                    indexChanged = true;
                    if(lastEventBatchTimestampIndex == ebtSize){ // we caught up to the current event
                        useEventTimeStorageHead = true;
                    }
                }
            }

            if(timeSincePlaybackReset < eventBatchTimestamps[0].second){//Data might come in so fast it overwrites 
                    //circular buffer and timeSincePlayback falls behind the timestamp of the tail element in eventBatchTimestamps
                playbackIndex = 0;
            }
        }

        //Don't render if the index has not changed
        if(!indexChanged)
            return;

        std::pair<std::unique_ptr<FEEvents>, std::unique_ptr<TrackData>>& batchData = eventBatchTimestamps[playbackIndex].first;
        std::unique_ptr<TrackData>& trackData = batchData.second;

        //Update hits on chips
        for(int i = 0; i < m_cli->getTotalFEs(); i++) {
            std::unique_ptr<EventData>& chipEventData = (*batchData.first)[i];

            if(chipEventData && chipEventData->size() > 0){
                glm::vec3 chipScale = m_chips[i].scale;

                Chip* currChip = &m_chips[i];
                float hitSize = (1.0f / std::min(currChip->maxRows, currChip->maxCols)) * currChip->scale[0];

                //Batch all of the hit data together to reduce number of calls to updateHitMap, which swaps shaders
                //Shader swapping is on order of magnitude of microseconds whereas moving/copying is nanoseconds
                std::vector<pixelHit> hitData;
                hitData.reserve(chipEventData->nHits);
                for(Event& currEvents : chipEventData->events){
                    for(auto hit : currEvents.hits){
                        std::uint16_t row = hit.row;
                        std::uint16_t col = hit.col;

                        if(row > m_chips[i].maxRows || col > m_chips[i].maxCols)
                            break;
                            
                        float diffy = ((float)row / (float)(currChip->maxRows)) * 2.0f - 1.0f;
                        diffy *= currChip->scale[1];

                        float diffx = ((float)col / (float)(currChip->maxCols)) * 2.0f - 1.0f;
                        diffx *= currChip->scale[0];
                        glm::vec3 posRelToChip = glm::vec3(diffx, diffy, 0.0f);
                        glm::vec3 pos = currChip->pos + glm::toMat3(glm::quat(glm::vec3(viz_TO_RADIANS(currChip->eulerRot[0]), viz_TO_RADIANS(currChip->eulerRot[1]), viz_TO_RADIANS(currChip->eulerRot[2])))) * posRelToChip; //this could probably be optimized for later

                        Particle tempPart;
                        tempPart.pos = pos;
                        tempPart.transform = transform(glm::vec3(hitSize, hitSize, currChip->scale[2] + 0.1f), currChip->eulerRot, pos, currChip->orientation, currChip->usingRads);
                        tempPart.color = glm::vec4(defaultHitColor, 1.0f);
                        tempPart.is_immortal = false;
                        tempPart.lifetime = hitDuration;
                        tempPart.ndcDepth = 0.0f;
                        tempPart.isHit = true;
                        ParticlesContainer[findUnusedParticle()] = tempPart;

                        currChip->hits += 1;

                        system::ParticleHit hitEvent(currChip->name, currChip->hits, row, col);
                        eventCallback(hitEvent);

                        hitData.emplace_back(row, col);
                    }
                }

                currChip->updateHitMap(hitData);
            }
        }

        //Update tracks
        if(trackData && trackData->size() > 0){
            for(auto& track : *trackData){
                if(track->trackType == Track::type::straight){
                    StraightLineTrack* straight = static_cast<StraightLineTrack*>(track.get());
                    //Length of the track is determined by the length of the detector divded by cos of the angle between track and z-axis
                    //float trackLength = (detectorLength * straight->direction.length()) / (glm::dot(straight->direction, glm::vec3(0.0f, 0.0f, 1.0f)));
                    float trackLength = 500.0f; // Need to change so not harcoded
                    glm::vec3 size(0.05f, 0.05f, trackLength);

                    //Create quat that rotates from +Z-dir to direction
                    glm::normalize(straight->direction);
                    const glm::vec3 zAxis{0.f,0.f,1.f};
                    float dot       = glm::dot(zAxis, straight->direction);
                    glm::vec3 cross = glm::cross(zAxis, straight->direction);

                    if (glm::length2(cross) < 1e-6f) {
                        cross = glm::vec3{1.f,0.f,0.f};
                        dot   = -1.f;
                    }

                    glm::quat q{ dot + 1.f, cross.x, cross.y, cross.z };
                    q = glm::normalize(q);

                    Particle trackPart;
                    glm::vec4 color;

                    if(type == 0){
                        color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                        type++;
                    }
                    else if(type ==1){
                        color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
                        type++;
                    }
                    else if(type == 2){
                        color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
                        type = 0;
                    }
                    

                    trackPart.pos = straight->point;
                    trackPart.transform = transform(size, q, straight->point);
                    //trackPart.color = color;
                    trackPart.color = glm::vec4(0.745f, 0.373f, 0.859f, 1.0f);
                    trackPart.is_immortal = trackIsImmortal;
                    trackPart.lifetime = hitDuration;
                    trackPart.ndcDepth = 0.0f;
                    trackPart.isHit = false;
                    ParticlesContainer[findUnusedParticle()] = trackPart;
                }
            }
        }

        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;

        totalupdatetime += std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
        totalupdateframes++;
    }

    void Detector::updateHitDurations(float dur){
        hitDuration = dur;
        for(int i = 0; i < ParticlesContainer.size(); i++){
            if(ParticlesContainer[i].lifetime > dur)
                ParticlesContainer[i].lifetime = dur;
        }
    }

    void Detector::render(const Shader& shader){
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        SimpleCubeMesh.render(shader);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void Detector::updateParticles(const Camera& cam, const float& dTime){
        SimpleCubeMesh.m_instances.clear();
        for(int i = 0; i < ParticlesContainer.size(); i++){
            Particle& p = ParticlesContainer[i];

            if(!p.is_immortal){
                if(p.lifetime > 0.0f){
                    p.lifetime -= dTime;
                    if(p.lifetime > 0.0f){
                        glm::vec4 clip = cam.getProj() * cam.getView() * glm::vec4(p.pos, 1.0f);
                        float ndcDepth = clip.z / clip.w;
                        p.ndcDepth = ndcDepth;

                        InstanceData data;
                        data.transform = p.transform;
                        float ratio = p.lifetime / hitDuration;
                        if(p.isHit)
                            data.color = glm::vec4((1 - ratio), ratio, 0, p.lifetime / hitDuration);
                        else{
                            data.color = glm::vec4(p.color.x, p.color.y, p.color.z, p.lifetime / hitDuration);
                        }

                        SimpleCubeMesh.m_instances.push_back(data);
                    }
                    else{
                        p.ndcDepth = -1.0f;
                    }
                }
            }else{
                glm::vec4 clip = cam.getProj() * cam.getView() * glm::vec4(p.pos, 1.0f);
                float ndcDepth = clip.z / clip.w;
                p.ndcDepth = ndcDepth;

                InstanceData data;
                data.transform = p.transform;
                data.color = p.color;
                SimpleCubeMesh.m_instances.push_back(data);
            }
        }
        SimpleCubeMesh.updateInstances();
    }

    int Detector::findUnusedParticle(){
        //When not sorted, next available particle should be one after the last used particle
        //Unused particles have ndcDepth <= -1.0f, will accumulate at the end of list when sorted in sortTransparent
        for(int i=LastUsedParticle; i < totalParticles; i++){
            if (ParticlesContainer[i].lifetime <= 0.0f && !ParticlesContainer[i].is_immortal){
                    LastUsedParticle = i;
                    return i;
            }
        }

        for(int i=0; i<LastUsedParticle; i++){
            if (ParticlesContainer[i].lifetime <= 0.0f && !ParticlesContainer[i].is_immortal){
                LastUsedParticle = i;
                return i;
            }
        }

        for(int i = 0; i < totalParticles; i++){
            if(!ParticlesContainer[i].is_immortal)
                return i;
        }

        return -1;
    }

    void Detector::sortTransparent(const Camera& cam){
        // std::sort(ParticlesContainer.begin(), ParticlesContainer.end(), [](Particle& left, Particle& right){
        //     return left.ndcDepth > right.ndcDepth;
        // });
    }

    glm::mat4 Detector::transform(const glm::vec3& scale, const glm::quat& args_quat, const glm::vec3& pos){
        glm::mat4 tempTfm = glm::mat4(1.0f);
        tempTfm = glm::scale(tempTfm, scale);

        tempTfm = glm::toMat4(args_quat) * tempTfm;
        tempTfm = glm::translate(glm::mat4(1.0f), pos) * tempTfm;
        return globalDetectorTransformation * tempTfm;
    }
    

    glm::mat4 Detector::transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, OrientationMode orientation, bool isInRadians){
        glm::mat4 tempTfm = glm::mat4(1.0f);
        tempTfm = glm::scale(tempTfm, scale);

        if (!isInRadians) {
            eulerRot = glm::radians(eulerRot);
        }

        // Build rotation matrix based on extrinsic Euler angles
        glm::mat4 rot = glm::mat4(1.0f);
        switch (orientation) {
            case OrientationMode::XYZ:
                // Extrinsic: apply X, then Y, then Z rotation around global axes
                rot = glm::rotate(glm::mat4(1.0f), eulerRot.x, glm::vec3(1, 0, 0));
                rot = glm::rotate(rot, eulerRot.y, glm::vec3(0, 1, 0));
                rot = glm::rotate(rot, eulerRot.z, glm::vec3(0, 0, 1));
                break;

            case OrientationMode::ZYX:
                rot = glm::rotate(glm::mat4(1.0f), eulerRot.z, glm::vec3(0, 0, 1));
                rot = glm::rotate(rot, eulerRot.y, glm::vec3(0, 1, 0));
                rot = glm::rotate(rot, eulerRot.x, glm::vec3(1, 0, 0));
                break;

            case OrientationMode::ZXZ:
                rot = glm::rotate(glm::mat4(1.0f), eulerRot.z, glm::vec3(0, 0, 1));
                rot = glm::rotate(rot, eulerRot.x, glm::vec3(1, 0, 0));
                rot = glm::rotate(rot, eulerRot.z, glm::vec3(0, 0, 1));
                break;

            case OrientationMode::QUAT:
                rot = glm::toMat4(glm::quat(eulerRot));

            default:
                break;
        }

        tempTfm = rot * tempTfm;
        tempTfm = glm::translate(glm::mat4(1.0f), pos) * tempTfm;

        return globalDetectorTransformation * tempTfm;
    }
}