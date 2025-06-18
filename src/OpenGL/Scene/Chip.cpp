#include "OpenGL/Scene/Chip.h"

namespace viz{    
    Chip::Chip(bool enable_hitmap, uint16_t max_cols, uint16_t max_rows){
        maxCols = max_cols;
        maxRows = max_rows;

        hitMapEnabled = enable_hitmap;
        if(hitMapEnabled)
            enableHitMap();
        else{
            disableHitMap();
        }
    }

    void Chip::enableHitMap(){
        m_hitMapFramebuffer = std::make_unique<Framebuffer>(maxCols, maxRows);

        m_hitMapFramebuffer->bind();
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        m_hitMapFramebuffer->unbind();

        glGenVertexArrays(1, &hitMapVAO);
        glGenBuffers(1, &hitMapVBO);
        glBindVertexArray(hitMapVAO);
        glBindBuffer(GL_ARRAY_BUFFER, hitMapVBO);
        glEnableVertexAttribArray(0);

        hitMapEnabled = true;
    }

    void Chip::disableHitMap(){
        m_hitMapFramebuffer = nullptr;

        if(hitMapEnabled){
            glDeleteVertexArrays(1, &hitMapVAO);
            glDeleteBuffers(1, &hitMapVBO);
        }
        hitMapEnabled = false;
    }

    void Chip::updateHitMap(const std::vector<pixelHit>& hits){
        if(!hitMapEnabled || hits.empty()) return;

        //convert pixel coordinates to normalized device coordinates [-1, 1]
        std::vector<float> points;
        for(const auto& hit : hits) {
            float x = 2.0f * ((float)hit.col + 0.5f) / ((float)maxCols) - 1.0f;
            float y = 2.0f * ((float)hit.row + 0.5f) / ((float)maxRows) - 1.0f;
            points.push_back(x);
            points.push_back(y);
        }

        m_hitMapFramebuffer->bind();

        auto hitMapShader = ShaderManager::get("hitmap");
        hitMapShader->use();
        hitMapShader->setFloat("intensity", 1.0f); 

        glBindVertexArray(hitMapVAO);
        glBindBuffer(GL_ARRAY_BUFFER, hitMapVBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        //enable blending for adding hits
        glEnable(GL_BLEND);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawArrays(GL_POINTS, 0, points.size() / 2);

        glDisable(GL_BLEND);
        glBindVertexArray(0);

        m_hitMapFramebuffer->unbind();
    }
}
