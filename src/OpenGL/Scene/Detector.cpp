#include "OpenGL/Scene/Detector.h"

#include <random>

namespace {
    auto log = logging::make_log("Detector");
}

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

    std::vector<SimpleVertex> HitVertices = {
        // Front Vertices //Color
        {glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        // Back Vertices  //Color
        {glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)},
        {glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.2509f, 0.0235f, 0.95f)}
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
        ParticlesContainer.resize(10000);
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

            Chip tempChip;
            tempChip.name = name;
            tempChip.fe_id = i;
            tempChip.pos = glm::vec3(pos[0], pos[1], pos[2]);
            tempChip.eulerRot = glm::vec3(angle[0], angle[1], angle[2]);
            tempChip.scale = glm::vec3(size[0] / 2.0f, size[1] / 2.0f, size[2] / 2.0f);
            tempChip.maxRows = rowcol[0]; tempChip.maxCols = rowcol[1];
            tempChip.hits = 0;

            glm::mat4 tempTfm = transform(tempChip.scale, tempChip.eulerRot, tempChip.pos, false);
            
            m_chips.push_back(tempChip);
            
            Particle tempPart;
            tempPart.pos = tempChip.pos;
            tempPart.transform = tempTfm;
            tempPart.color = glm::vec4(0.3921f, 0.3921f, 0.3921f, 0.25f);
            tempPart.is_immortal = true;
            tempPart.lifetime = 100.0f;
            tempPart.ndcDepth = 0.0f;
            ParticlesContainer[FindUnusedParticle()] = tempPart;
        };
        
        m_appLogger->info("Detector initialized with {0} chips", m_chips.size());
    }

    void Detector::update(const Camera& cam, float dTime){
        if(!m_cli->isRunning())
            return;
        
        m_nfe = 0;
        m_size = 0;

        std::this_thread::sleep_for(std::chrono::nanoseconds(25));
        for(int i = 0; i < m_cli->getTotalFEs(); i++) {
            std::unique_ptr<std::vector<pixelHit>> data = m_cli->getData(i, true);
            if(data && data->size() > 0){
                glm::vec3 chipScale = m_chips[i].scale;

                Chip* currChip = &m_chips[i];
                float hitSize = (1.0f / std::min(currChip->maxRows, currChip->maxCols)) * currChip->scale[0];
                m_size += data->size();
                nHits += data->size();
                
                for(int j = 0; j < data->size(); j++){
                    std::uint16_t row = (*data)[j].row;
                    std::uint16_t col = (*data)[j].col;

                    if(row > m_chips[i].maxRows || col > m_chips[i].maxCols)
                        break;
                    
                    float diffx = (2.0f * ((float)row / (float)currChip->maxRows) - 1.0f) * currChip->scale[0];
                    float diffy = (2.0f * ((float)col / (float)currChip->maxCols) - 1.0f) * currChip->scale[1];
                    glm::vec3 posRelToChip =  glm::vec3(diffx, diffy, 0.0f);
                    glm::vec3 pos = currChip->pos + glm::toMat3(glm::quat(glm::vec3(viz_TO_RADIANS(currChip->eulerRot[0]), viz_TO_RADIANS(currChip->eulerRot[1]), viz_TO_RADIANS(currChip->eulerRot[2])))) * posRelToChip; //this could probably be optimized for later

                    //CubeMesh.m_instances.emplace_back(defaultHitColor, transform(glm::vec3(hitSize, hitSize, currChip->scale[2] + 0.1f), currChip->eulerRot, pos));
                    Particle tempPart;
                    tempPart.pos = pos;
                    tempPart.transform = transform(glm::vec3(hitSize, hitSize, currChip->scale[2] + 0.1f), currChip->eulerRot, pos);
                    tempPart.color = glm::vec4(defaultHitColor, 1.0f);
                    tempPart.is_immortal = false;
                    tempPart.lifetime = particleLifetime;
                    tempPart.ndcDepth = 0.0f;
                    ParticlesContainer[FindUnusedParticle()] = tempPart;

                    currChip->hits += 1;

                    ParticleHit hitEvent(currChip->name, currChip->hits, row, col);
                    eventCallback(hitEvent);
                }

                data.reset();
                m_nfe++;
            }
        }
        //CubeMesh.updateInstances();
        
        CubeMesh.m_instances.clear();
        for(int i = 0; i < ParticlesContainer.size(); i++){
            Particle& p = ParticlesContainer[i];

            if(!p.is_immortal){
                if(p.lifetime > 0.0f);{
                    p.lifetime -= dTime;
                    if(p.lifetime > 0.0f){
                        glm::vec4 clip = cam.getProj() * cam.getView() * glm::vec4(p.pos, 1.0f);
                        float ndcDepth = clip.z / clip.w;
                        p.ndcDepth = ndcDepth;

                        InstanceData data;
                        data.transform = p.transform;
                        float ratio = p.lifetime / particleLifetime;
                        data.color = glm::vec4((1 - ratio), ratio, 0, p.lifetime / particleLifetime);

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
        std::cout << ParticlesContainer[900].lifetime << std::endl;
        CubeMesh.updateInstances();

        // m_cli->state = CLIstate::RECONSTRUCT;
        // std::this_thread::sleep_for(std::chrono::nanoseconds(25));
        // auto test = m_cli->getReconstructedBunch();
        // for(int i = 0; i < test->size(); i++){
        //     nHits += (*test)[i].nHits;
        // }
        // std::cout << "Detector hits total: " << nHits << std::endl;

    }

    void Detector::render(const Shader& shader){
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        CubeMesh.render(shader);
        glDepthMask(GL_TRUE);
    }

    int Detector::FindUnusedParticle(){
        for(int i=LastUsedParticle; i < 10000; i++){
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

        for(int i = 0; i < 10000; i++){
            if(!ParticlesContainer[i].is_immortal)
                return i;
        }
    }

    void Detector::sortTransparent(const Camera& cam){
        std::sort(ParticlesContainer.begin(), ParticlesContainer.end(), [](Particle& left, Particle& right){
            return left.ndcDepth > right.ndcDepth;
        });

    }

    // void Detector::sortTransparent(const Camera& cam){
    //     std::vector<std::pair<unsigned int, float>> perm(m_chips.size());
    //     for(int i = 0; i < m_chips.size(); i++){
    //         glm::vec4 clip = cam.getProj() * cam.getView() * glm::vec4(m_chips[i].pos, 1.0f);
    //         float ndcDepth = clip.z / clip.w;
    //         perm[i] = std::make_pair(i, ndcDepth);
    //     }
    //     std::sort(perm.begin(), perm.end(), [](std::pair<unsigned int, float>& left, std::pair<unsigned int, float>& right){
    //         return left.second > right.second;
    //     });
    //     std::vector<glm::mat4> secondMat = m_chipTransforms;
    //     for(int i = 0; i < perm.size(); i++){
    //         m_chipTransforms[i] = secondMat[perm[i].first];
    //     }
    //     std::vector<glm::vec4> secondVec = m_chipColors;
    //     for(int i = 0; i < perm.size(); i++){
    //         m_chipColors[i] = secondVec[perm[i].first];
    //     }

    //     ChipMesh.setInstances(m_chips.size(), m_chipTransforms, m_chipColors);
    // }

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