#ifndef CURVE_H
#define CURVE_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <functional>

#include "util/include/logging.h"

namespace viz{

    enum CurveType{
        BEZIER, BSPLINE, ARC, HELIX //TODO: Add additional curves
    };

    //Curves are parameterized from [0,1]
    class Curve{
        public:
            virtual std::vector<glm::vec3> resolvePoints() const = 0; //returns ordered list of points to render curve with straight lines
            virtual glm::vec3 at(float time) const = 0;  //get vector at provided normalized time
            virtual glm::vec3 derivative(float time) const = 0;

            void initForRender();
            void render() const;
            void changeResolution(float resolution);

            CurveType getType() { return type; }

            std::vector<glm::vec3> points;
            glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        protected:
            float timestep;
            bool isInit = false;
            unsigned int VAO, VBO;
            unsigned int totalVertices;
            CurveType type;
    };

    class Bezier : public Curve{
        public:
            Bezier() = delete;

            //Use de Casteljau's algorithm for numerically stable higher-order Bezier curves.
            //For cubic/quadratic Bezier curves set use_casteljau for more performance when evaluating.
            Bezier(const std::vector<glm::vec3>& control_points, bool use_casteljau, float resolution = 0.01f);

            std::vector<glm::vec3> resolvePoints() const override;
            glm::vec3 at(float time) const override;
            glm::vec3 derivative(float t) const override;
    
        private:
            glm::vec3 casteljau(const std::vector<glm::vec3>& points, const float& t) const;

            int totalPoints;
            std::vector<float> coeffs;
            std::function<float(float, int, int)> bk_n = [](float t, int k, int n){ return pow(t, k) * pow(1.0f-t, n-k); }; //Bernstein polynomials without coeff
            bool useCasteljau;
            float timestep;
    };

    class BSpline : public Curve {
        public:
            BSpline() = delete;

            BSpline(const std::vector<glm::vec3>& control_points, int degree = 3, float resolution = 0.01f);
            std::vector<glm::vec3> resolvePoints() const override;
            glm::vec3 derivative(float t) const override;
            glm::vec3 at(float t) const override ;
            
        private:
            float basis(int i, int k, float t) const;
            std::vector<float> knots;
            int degree;

            int findKnotSpan(float u) const;
    };

    class Arc : public Curve {
        public:
            Arc(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, unsigned int segments = 32);

            glm::vec3 at(float t) const override;
            std::vector<glm::vec3> resolvePoints() const override;
            glm::vec3 derivative(float t) const override;

            void changeResolution(float step);

        private:
            glm::vec3 control0, control1, control2;
            glm::vec3 center;
            glm::vec3 normal;
            glm::vec3 u, v;
            float radius;
            float thetaStart, thetaMid, thetaEnd;
        };

    class Helix : public Curve {
        public:
            Helix(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, unsigned int segments = 64, float coils = 1.0f);

            glm::vec3 at(float t) const override;
            glm::vec3 derivative(float t) const override;
            std::vector<glm::vec3> resolvePoints() const override;

        private:
            glm::vec3 start, control, end;
            unsigned int numSegments;
            float radius = 1.0f;
            float pitch = 1.0f;
            float totalTurns = 3.0f;
            glm::mat4 helixTransform = glm::mat4(1.0f);

            void computeParameters();
    };

}

#endif