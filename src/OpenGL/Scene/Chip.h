#ifndef CHIP_H
#define CHIP_H

#include "core/header.h"
#include "datasets/include/DataBase.h"
#include "OpenGL/TextureArray.h"
#include "OpenGL/ShaderManager.h"
#include "OpenGL/Framebuffer.h"
#include "OpenGL/ShaderManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace viz{
    class Chip {
        public:
            Chip() = delete;
            Chip(bool enable_hitmap, uint16_t max_cols, uint16_t max_rows);

            void enableHitMap();
            void disableHitMap();
            void updateHitMap(const std::vector<pixelHit>& hits);

            // Getters for basic attributes
            inline bool isHitMapEnabled() const { return hitMapEnabled; }
            inline std::string getName() const { return name; }
            inline std::uint16_t getFeId() const { return fe_id; }
            inline std::uint16_t getMaxCols() const { return maxCols; }
            inline std::uint16_t getMaxRows() const { return maxRows; }
            inline std::uint64_t getHits() const { return hits; }
        
            // Getters for 3D properties
            inline glm::vec3 getPosition() const { return pos; }
            inline glm::vec3 getRotation() const { return eulerRot; }
            inline glm::vec3 getScale() const { return scale; }

            std::shared_ptr<Texture2D> getTexture() { return m_hitMapFramebuffer->getTexture(); } 

            std::string name;
            std::uint16_t fe_id, maxCols, maxRows;
            std::uint64_t hits;
        
            glm::vec3 pos, eulerRot, scale;
        private:
            friend class Detector;
            std::unique_ptr<Framebuffer> m_hitMapFramebuffer;
            GLuint hitMapVAO, hitMapVBO;
            bool hitMapEnabled;
        };
}

#endif