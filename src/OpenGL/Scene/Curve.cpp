#include "OpenGL/Scene/Curve.h"

#include <iostream>
namespace viz{

    namespace{
        auto logger = logging::make_log("Curve");
    }

    void Curve::initForRender(){
        if(isInit){
            return;
        }

        std::vector<glm::vec3> discrete_points = resolvePoints();
        totalVertices = discrete_points.size();
        
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * discrete_points.size(), discrete_points.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        isInit = true;
    }

    void Curve::render() const{
        if(isInit){
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINE_STRIP, 0, totalVertices);
            glBindVertexArray(0);
        }
    }

    void Curve::changeResolution(float resolution) {
        timestep = glm::clamp(resolution, 0.0001f, 1.0f);
        std::vector<glm::vec3> discrete_points = resolvePoints();
        totalVertices = discrete_points.size();

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * discrete_points.size(), discrete_points.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    Bezier::Bezier(const std::vector<glm::vec3>& control_points, bool use_casteljau, float resolution){
        type = CurveType::BEZIER;
        timestep = resolution;

        if(control_points.size() < 3){
            logger->error("Bezier curve needs at least three control points. Only {0} provided.", control_points.size());
        }

        //Compute combinatorial part of bernstein polynomials for later use, saving computation
        auto factorial = [](int n) { int total = 1; for(int i = 2; i <= n; i++) { total *= i; } return total;};

        int n = control_points.size();
        totalPoints = n;
        points.reserve(n);
        coeffs.reserve(n);
        for(int k = 0; k < n; k++){
            points.push_back(control_points[k]);
            coeffs.push_back(factorial(n-1) / (factorial(n-1-k) * factorial(k)));
        }
    }

    std::vector<glm::vec3> Bezier::resolvePoints() const{
        std::vector<glm::vec3> allPoints;
        for(float time = 0.0f; time <= 1.0f; time += timestep){
            allPoints.push_back(at(time));
        }

        return allPoints;
    }

    glm::vec3 Bezier::at(float time) const{
        time = glm::clamp(time, 0.0f, 1.0f);
        glm::vec3 point;
        if(useCasteljau){
            point = casteljau(points, time);
        }
        else{
            point = glm::vec3(0.0f, 0.0f, 0.0f);
            int n = totalPoints;
            for(int k = 0; k < n; k++){
                point += coeffs[k] * bk_n(time, k, n) * points[k];
            }
        }
        return point;
    }

    glm::vec3 Bezier::casteljau(const std::vector<glm::vec3>& points, const float& t) const{
        if(points.size() == 2){
            return t * points[1] + (1.0f - t) * points[0];
        }

        std::vector<glm::vec3> newPoints;
        newPoints.reserve(points.size() - 1);
        for(int i = 0; i < points.size() - 1; i++){
            newPoints.push_back(t * points[i+1] + (1.0f - t) * points[i]);
        }
        
        return casteljau(newPoints, t);
    }

    BSpline::BSpline(const std::vector<glm::vec3>& control_points, int degree, float resolution) {
        type = CurveType::BSPLINE;
        points = control_points;
        this->degree = degree;
        timestep = resolution;

        int n = static_cast<int>(points.size());

        int m = n + degree + 1;
        knots.resize(m);
        for (int i = 0; i <= degree; ++i) knots[i] = 0.0f;

        int interior = m - 2*(degree+1);
        for (int j = 1; j <= interior; ++j) {
            knots[degree + j] = float(j) / float(interior + 1);
        }

        for (int i = m - degree - 1; i < m; ++i) knots[i] = 1.0f;
    }

    std::vector<glm::vec3> BSpline::resolvePoints() const {
        std::vector<glm::vec3> samples;
        for (float t = 0.0f; t <= 1.0f; t += timestep) {
            samples.push_back(at(t));
        }
        // Ensure last point at t = 1.0
        samples.push_back(at(1.0f));
        return samples;
    }

    glm::vec3 BSpline::at(float t) const {
        float u = glm::clamp(t, 0.0f, 1.0f);
        int k = findKnotSpan(u);

        std::vector<glm::vec3> d(degree+1);
        for (int j = 0; j <= degree; ++j) {
            d[j] = points[k - degree + j];
        }

        //Implement with De Boor algorithm
        for (int r = 1; r <= degree; ++r) {
            for (int j = degree; j >= r; --j) {
                float alpha = (u - knots[k - degree + j]) / (knots[j + k - r + 1] - knots[k - degree + j]);
                d[j] = (1.0f - alpha) * d[j-1] + alpha * d[j];
            }
        }
        return d[degree];
    }

    glm::vec3 Bezier::derivative(float t) const {
        int n = points.size() - 1;
        if (n < 1) return glm::vec3(0.0f);

        std::vector<glm::vec3> diffPoints(n);
        for (int i = 0; i < n; ++i) {
            diffPoints[i] = float(n) * (points[i + 1] - points[i]);
        }

        if (useCasteljau) {
            return casteljau(diffPoints, t);
        } else {
            glm::vec3 result(0.0f);
            for (int i = 0; i < n; ++i) {
                float coeff = coeffs[i] * float(n) * (bk_n(t, i, n - 1) - bk_n(t, i - 1, n - 1));
                result += coeff * points[i];
            }
            return result;
        }
    }

    glm::vec3 BSpline::derivative(float t) const {
        float u = t * (knots.back() - knots.front()) + knots.front();
        float du = knots.back() - knots.front();
        glm::vec3 sum(0.0f);
        int m = points.size();
        for(int i = 0; i < m - 1; ++i) {
            float denom1 = knots[i+degree]   - knots[i];
            float denom2 = knots[i+degree+1] - knots[i+1];
            float term1 = denom1>0 ? basis(i,   degree-1, u)/denom1 : 0.0f;
            float term2 = denom2>0 ? basis(i+1, degree-1, u)/denom2 : 0.0f;
            sum += degree * (term1 - term2) * points[i];
        }
        return sum * du;
    }

    float BSpline::basis(int i, int k, float t) const {
        if (k == 0) {
            return (t >= knots[i] && t < knots[i + 1]) ? 1.0f : 0.0f;
        }

        float denom1 = knots[i + k] - knots[i];
        float denom2 = knots[i + k + 1] - knots[i + 1];
        float term1 = 0.0f;
        float term2 = 0.0f;

        if (denom1 != 0.0f)
            term1 = (t - knots[i]) / denom1 * basis(i, k - 1, t);
        if (denom2 != 0.0f)
            term2 = (knots[i + k + 1] - t) / denom2 * basis(i + 1, k - 1, t);

        return term1 + term2;
    }


    int BSpline::findKnotSpan(float u) const {
        int m = static_cast<int>(knots.size());
        int n = static_cast<int>(points.size());

        if (u <= knots[degree]) return degree;
        if (u >= knots[n]) return n - 1;

        int low = degree;
        int high = n;
        int mid = (low + high) / 2;
        while (u < knots[mid] || u >= knots[mid+1]) {
            if (u < knots[mid]) high = mid;
            else low = mid;
            mid = (low + high) / 2;
        }
        return mid;
    }

    Arc::Arc(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, unsigned int segments){
        type = CurveType::ARC;

        control0 = p0;
        control1 = p1;
        control2 = p2;
        timestep = 1.0f / (float)segments;

        glm::vec3 v1 = p1 - p0;
        glm::vec3 v2 = p2 - p0;
        normal = glm::normalize(glm::cross(v1, v2));

        glm::vec3 m01 = 0.5f * (p0 + p1);
        glm::vec3 m12 = 0.5f * (p1 + p2);
        glm::vec3 d01 = glm::cross(p1 - p0, normal);
        glm::vec3 d12 = glm::cross(p2 - p1, normal);

        glm::vec3 absN = glm::abs(normal);
        int dropAxis = (absN.x > absN.y ? (absN.x > absN.z ? 0 : 2) : (absN.y > absN.z ? 1 : 2));

        auto project2 = [&](const glm::vec3& v) {
            if (dropAxis == 0) return glm::vec2(v.y, v.z);
            if (dropAxis == 1) return glm::vec2(v.x, v.z);
            return glm::vec2(v.x, v.y);
        };

        glm::vec2 r_m01 = project2(m01);
        glm::vec2 r_m12 = project2(m12);
        glm::vec2 r_d01 = project2(d01);
        glm::vec2 r_d12 = project2(d12);

        glm::mat2 A(r_d01, -r_d12);
        glm::vec2 b = r_m12 - r_m01;
        glm::vec2 st = glm::inverse(A) * b;
        center = m01 + st.x * d01;

        radius = glm::length(p0 - center);

        u = glm::normalize(p0 - center);
        v = glm::normalize(glm::cross(normal, u));

        thetaStart = 0.0f;
        thetaMid = atan2(glm::dot(p1 - center, v), glm::dot(p1 - center, u));
        thetaEnd = atan2(glm::dot(p2 - center, v), glm::dot(p2 - center, u));

        if (thetaEnd < 0) thetaEnd += glm::two_pi<float>();
        if (thetaMid < 0) thetaMid += glm::two_pi<float>();

        if (thetaMid > thetaEnd) {
            thetaEnd += glm::two_pi<float>();
        }
    }

    glm::vec3 Arc::at(float t) const {
        float theta = glm::mix(thetaStart, thetaEnd, t);
        return center + radius * (glm::cos(theta) * u + glm::sin(theta) * v);
    }

    glm::vec3 Arc::derivative(float t) const {
        float dTheta = thetaEnd - thetaStart;
        float theta = thetaStart + t * dTheta;
        glm::vec3 tangent = -glm::sin(theta) * u + glm::cos(theta) * v;
        return radius * dTheta * tangent;
    }

    std::vector<glm::vec3> Arc::resolvePoints() const {
        std::vector<glm::vec3> pts;
        for (float t = 0.0f; t <= 1.0f; t += timestep) {
            pts.push_back(at(t));
        }
        if (pts.back() != control2) pts.push_back(control2);
        return pts;
    }


    Helix::Helix(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, unsigned int segments, float coils) {
        start = p0;
        control = p1;
        end = p2;
        numSegments = segments;
        totalTurns = coils;
        type = CurveType::HELIX;
        computeParameters();
        timestep = 1.0f / static_cast<float>(numSegments);
        initForRender();
    }

    glm::vec3 Helix::at(float t) const {
        float theta = t * totalTurns * glm::two_pi<float>();
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        float z = pitch * theta / glm::two_pi<float>();
        glm::vec3 posLocal(x, y, z);
        return helixTransform * glm::vec4(posLocal, 1.0f);
    }

    glm::vec3 Helix::derivative(float t) const {
        float theta = t * totalTurns * glm::two_pi<float>();
        float dtheta_dt = totalTurns * glm::two_pi<float>();
        float dx_dt = -radius * sin(theta) * dtheta_dt;
        float dy_dt = radius * cos(theta) * dtheta_dt;
        float dz_dt = pitch * dtheta_dt / glm::two_pi<float>();
        glm::vec3 derivLocal(dx_dt, dy_dt, dz_dt);
        return glm::mat3(helixTransform) * derivLocal;
    }

    std::vector<glm::vec3> Helix::resolvePoints() const {
        std::vector<glm::vec3> pts;
        pts.reserve(numSegments + 1);
        for (unsigned int i = 0; i <= numSegments; ++i) {
            float t = i * timestep;
            pts.push_back(at(t));
        }
        return pts;
    }

    void Helix::computeParameters() {
        glm::vec3 axis = glm::normalize(end - start);
        float length = glm::length(end - start);
        glm::vec3 startToControl = control - start;
        glm::vec3 proj = glm::dot(startToControl, axis) * axis;
        glm::vec3 radiusVec = startToControl - proj;
        radius = glm::length(radiusVec);
        pitch = length / totalTurns;
        glm::vec3 zAxis = axis;
        glm::vec3 arbitrary = glm::vec3(0, 0, 1);
        if (glm::abs(glm::dot(zAxis, arbitrary)) > 0.99f)
            arbitrary = glm::vec3(0, 1, 0);
        glm::vec3 xAxis = glm::normalize(glm::cross(arbitrary, zAxis));
        glm::vec3 yAxis = glm::cross(zAxis, xAxis);
        helixTransform = glm::mat4(
            glm::vec4(xAxis, 0.0f),
            glm::vec4(yAxis, 0.0f),
            glm::vec4(zAxis, 0.0f),
            glm::vec4(start, 1.0f)
        );
        helixTransform = glm::transpose(helixTransform);
    }
}