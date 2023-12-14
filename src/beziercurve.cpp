#include "beziercurve.h"
#include <iostream>
#include <glm/glm.hpp>


BezierCurve::BezierCurve() {}

class BezierCurve {
public:

    void BezierCurve::setControlPoints(const glm::vec3& start, const glm::vec3& control1, const glm::vec3& control2, const glm::vec3 end) {
        p0 = start;
        p1 = control1;
        p2 = control2;
        p3 = end;
    }

    glm::vec3 BezierCurve::calculatePoint(float t) {

        float u = 1 - t;
        float tt = t*t;
        float uu = u*u;
        float uuu = uu * u;
        float ttt = tt * t;

        glm::vec3 p = uuu * p0;
        p += 3 * uu * t * p1;
        p += 3 * u * tt * p2;
        p += ttt * p3;

        return p;
    }


private:
    glm::vec3 p0, p1, p2, p3;
};
