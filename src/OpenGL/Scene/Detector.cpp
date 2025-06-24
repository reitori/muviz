#include "OpenGL/Scene/Detector.h"

namespace {
    auto logger = logging::make_log("Detector");
}

//TODO: Move matrix computations to GPU. Main bottleneck right now is the amount of CPU computation for particle rendering.

namespace viz{
    std::vector<SimpleVertex> CubeVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f,0.05f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.05f)}
    };

    std::vector<GLuint> CubeIndices = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    
    Detector::Detector(){
        CubeMesh = SimpleMesh(CubeVertices, CubeIndices, true);
        ParticlesContainer.resize(totalParticles);
        m_cli; 
    }

    void Detector::init(const std::shared_ptr<VisualizerCli>& cli){
        m_cli = cli;

        for(int i = 0; i < cli->getTotalFEs(); i++) {
            const json& config = cli->getConfig(i);
            std::string name = config["name"].get<std::string>();
            std::vector<float> pos = config["position"].get<std::vector<float>>();
            std::vector<float> angle = config["angle"].get<std::vector<float>>();
            std::vector<float> size = config["size"].get<std::vector<float>>();
            std::vector<float> rowcol = config["rowcol"].get<std::vector<float>>();

            m_chips.emplace_back(false, rowcol[1], rowcol[0]);
            auto& currChip = m_chips.back();

            currChip.name = name;
            currChip.fe_id = i;
            currChip.pos = glm::vec3(pos[0], pos[1], pos[2]);
            currChip.eulerRot = glm::vec3(angle[0], angle[1], angle[2]);
            currChip.scale = glm::vec3(size[0] / 2.0f, size[1] / 2.0f, size[2] / 2.0f);
            currChip.hits = 0;

            glm::mat4 tempTfm = transform(currChip.scale, currChip.eulerRot, currChip.pos, false);
            
            Particle tempPart;
            tempPart.pos = currChip.pos;
            tempPart.transform = tempTfm;
            tempPart.color = glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.25f);
            tempPart.is_immortal = true;
            tempPart.lifetime = 100.0f;
            tempPart.ndcDepth = 0.0f;
            ParticlesContainer[findUnusedParticle()] = tempPart;
        };
        
        logger->info("Detector initialized with {0} chips", m_chips.size());
    }

    void Detector::update(const Camera& cam, const float& dTime){
        if(!m_cli->isRunning())
            return;

        auto start = std::chrono::steady_clock::now();
        for(int i = 0; i < m_cli->getTotalFEs(); i++) {
            std::unique_ptr<std::vector<pixelHit>> data = m_cli->getData(i, true);
            if(data && data->size() > 0){
                glm::vec3 chipScale = m_chips[i].scale;

                Chip* currChip = &m_chips[i];
                float hitSize = (1.0f / std::min(currChip->maxRows, currChip->maxCols)) * currChip->scale[0];
                nHitsThisFrame += data->size();
                
                currChip->updateHitMap(*data);

                for(int j = 0; j < data->size(); j++){
                    std::uint16_t row = (*data)[j].row;
                    std::uint16_t col = (*data)[j].col;

                    if(row > m_chips[i].maxRows || col > m_chips[i].maxCols)
                        break;
                    
                    float diffx = (2.0f * (((float)row + 0.5f) / (float)currChip->maxRows) - 1.0f) * currChip->scale[0];
                    float diffy = (2.0f * (((float)col + 0.5f) / (float)currChip->maxCols) - 1.0f) * currChip->scale[1];
                    glm::vec3 posRelToChip = glm::vec3(diffx, diffy, 0.0f);
                    glm::vec3 pos = currChip->pos + glm::toMat3(glm::quat(glm::vec3(viz_TO_RADIANS(currChip->eulerRot[0]), viz_TO_RADIANS(currChip->eulerRot[1]), viz_TO_RADIANS(currChip->eulerRot[2])))) * posRelToChip; //this could probably be optimized for later

                    Particle tempPart;
                    tempPart.pos = pos;
                    tempPart.transform = transform(glm::vec3(hitSize, hitSize, currChip->scale[2] + 0.1f), currChip->eulerRot, pos);
                    tempPart.color = glm::vec4(defaultHitColor, 1.0f);
                    tempPart.is_immortal = false;
                    tempPart.lifetime = hitDuration;
                    tempPart.ndcDepth = 0.0f;
                    ParticlesContainer[findUnusedParticle()] = tempPart;

                    currChip->hits += 1;

                    ParticleHit hitEvent(currChip->name, currChip->hits, row, col);
                    eventCallback(hitEvent);
                }

                data.reset();
            }
        }
        
        updateParticles(cam, dTime);
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
        CubeMesh.render(shader);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void Detector::updateParticles(const Camera& cam, const float& dTime){
        CubeMesh.m_instances.clear();
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
                        data.color = glm::vec4((1 - ratio), ratio, 0, p.lifetime / hitDuration);

                        CubeMesh.m_instances.push_back(data);
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
                CubeMesh.m_instances.push_back(data);
            }
        }
        CubeMesh.updateInstances();
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

    glm::mat4 Detector::transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, bool isInRadians){
        glm::mat4 tempTfm = glm::mat4(1.0f);
        tempTfm = glm::scale(tempTfm, scale);

        if(!isInRadians)
            eulerRot = glm::vec3(viz_TO_RADIANS(eulerRot[0]), viz_TO_RADIANS(eulerRot[1]), viz_TO_RADIANS(eulerRot[2]));

        tempTfm = glm::toMat4(glm::quat(eulerRot)) * tempTfm;
        tempTfm = glm::translate(glm::mat4(1.0f), pos) * tempTfm;
        return tempTfm;
    }
}