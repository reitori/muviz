#include "OpenGL/Scene/Detector.h"

namespace viz{
    void Detector::init(const VisualizerCli& cli){
        for(int i = 0; i < cli.getSize(); i++) {
            const json& config = cli.getConfig(i);
            std::vector<std::string> name = config["name"].get<std::vector<std::string>>();
            std::vector<float> pos = config["position"].get<std::vector<float>>();
            std::vector<float> angle = config["angle"].get<std::vector<float>>();
            std::vector<float> size = config["size"].get<std::vector<float>>();
            std::vector<float> rowcol = config["rowcol"].get<std::vector<float>>();

            Chip tempChip;
            //tempChip.m_name = name[0];
            tempChip.fe_id = i;
            tempChip.pos = glm::vec3(pos[0], pos[1], pos[2]);
            tempChip.eulerRot = glm::vec3(angle[0], angle[1], angle[2]);
            tempChip.scale = glm::vec3(size[0] / 2.0f, size[1] / 2.0f, size[2] / 2.0f);
            tempChip.hits = 0;

            glm::mat4 tempTfm = transform(tempChip.scale, tempChip.eulerRot, tempChip.pos, false);
            
            m_chips.push_back(tempChip);
            m_chipTransforms.push_back(tempTfm);
            m_chipColors.push_back(glm::vec4(0.3921f, 0.3921f, 0.3921f, 1.0f));
        };
        ChipMesh.setInstances(m_chips.size(), m_chipTransforms, m_chipColors);
    }

    void Detector::update(VisualizerCli& cli){
        m_nfe = 0;
        m_size = 0;

        for(int i = 0; i < cli.getSize(); i++) {
            std::unique_ptr<std::vector<pixelHit>> data = cli.getData(i, true);
            glm::vec3 chipScale = m_chips[i].scale;
            if(data){
                m_size += data->size();
                m_chips[i].hits = data->size();
                Chip currChip = m_chips[i];
                
                for(int i = 0; i < data->size(); i++){
                    std::uint16_t row = (*data)[i].row;
                    std::uint16_t col = (*data)[i].col;

                    float diffx = (float)(row / currChip.maxRows) * (2.0f * currChip.scale[0]);
                    float diffy = (float)(col / currChip.maxCols) * (2.0f * currChip.scale[1]);

                    glm::vec3 posRelToChip =  glm::vec3(diffx, diffy, 0.0f) - currChip.scale;
                    m_Hits.push_back(transform(hitScale, currChip.eulerRot, currChip.pos + posRelToChip));
                }

                data.reset();
                m_nfe++;
            }
        }
    }

    void Detector::renderChips(const Shader& shader){
        ChipMesh.render(shader);
    }

    void Detector::renderHits(const Shader& shader){
        HitMesh.render(shader);
    }


    glm::mat4 Detector::transform(glm::vec3 scale, glm::vec3 eulerRot, glm::vec3 pos, bool isInRadians){
        glm::mat4 tempTfm = glm::mat4(1.0f);
        tempTfm = glm::scale(tempTfm, scale);

        if(!isInRadians)
            eulerRot = glm::vec3(viz_TO_RADIANS(eulerRot[0]), viz_TO_RADIANS(eulerRot[1]), viz_TO_RADIANS(eulerRot[2]));

        tempTfm = glm::rotate(tempTfm, eulerRot[0], glm::vec3(0.0f, 0.0f, 1.0f));
        tempTfm = glm::rotate(tempTfm, eulerRot[1], glm::vec3(0.0f, 1.0f, 0.0f));
        tempTfm = glm::rotate(tempTfm, eulerRot[2], glm::vec3(1.0f, 0.0f, 0.0f));

        tempTfm = glm::translate(tempTfm, pos);
        return tempTfm;
    }
}