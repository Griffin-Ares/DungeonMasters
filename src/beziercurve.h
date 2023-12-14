#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H
#include <glm/glm.hpp>

class BezierCurve
{
public:
    BezierCurve();

    glm::vec3 calculatePoints(float t);

    void setControlPoints(const glm::vec3& start, const glm::vec3& control1, const glm::vec3& control2, const glm::vec3 end);


private:
    glm::vec3 p0, p1, p2, p3;

};

#endif // BEZIERCURVE_H
