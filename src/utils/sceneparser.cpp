#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void dfsBuildCTM(SceneNode* node, const glm::mat4& currentCTM, RenderData &renderData) {
    glm::mat4 newCTM = currentCTM;

    // Apply each transformation in the node to our cumulative transformation matrix
    for (SceneTransformation* transformation : node->transformations) {
        switch (transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            newCTM = glm::translate(newCTM, transformation->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            newCTM = glm::scale(newCTM, transformation->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            newCTM = glm::rotate(newCTM, transformation->angle, transformation->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            newCTM = transformation->matrix * newCTM;
            break;
        }
    }

    for (ScenePrimitive* primitive : node->primitives) {
        RenderShapeData shape;
        shape.ctm = newCTM;

        shape.primitive.type = primitive->type;
        shape.primitive.material = primitive->material;
        shape.primitive.meshfile = primitive->meshfile;

        renderData.shapes.push_back(shape);
    }

    for (SceneLight* light : node->lights) {
        SceneLightData lightData;
        lightData.id = light->id;
        lightData.type = light->type;
        lightData.color = light->color;
        lightData.function = light->function;

        switch (light->type) {
        case LightType::LIGHT_POINT:
            lightData.pos = newCTM * glm::vec4(0.f, 0.f, 0.f, 1.f);
            break;
        case LightType::LIGHT_SPOT:
            lightData.pos = newCTM * glm::vec4(0.f, 0.f, 0.f, 1.f);
            lightData.dir = newCTM * light->dir;
            lightData.penumbra = light->penumbra;
            lightData.angle = light->angle;
            break;
        case LightType::LIGHT_DIRECTIONAL:
            lightData.dir = newCTM * light->dir;
            break;
        }

        renderData.lights.push_back(lightData);
    }

    // Continue the DFS by visiting the children
    for (SceneNode* child : node->children) {
        dfsBuildCTM(child, newCTM, renderData);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();

    // pass beginning ctm as identity matrix
    dfsBuildCTM(root, glm::mat4(1.0f), renderData);


    return true;
}
