#include "room.h"
#include "glm/gtx/transform.hpp"

/*
 *     RenderShapeData floor;
 *     RenderShapeData ceiling;
 *     std::vector<RenderShapeData> walls;
 */

Room::Room(RoomType type, float x, float z) : x(x), z(z) {
    // Initialize room components
    if (type == RoomType::Default) {
        generateFloor();
        generateCeiling();
    }
    //generateWalls(4);
}

bool Room::isEmpty() {
    return walls.empty();
}

void Room::generateWalls(glm::vec4 state) {
    // Assuming each wall is a scaled cube
    float wallLength = 5.0f; // Length of the wall
    float wallHeight = 3.0f; // Height of the wall
    float wallThickness = 0.2f; // Thickness of the wall
    float roomWidth = 2.5f;
    float roomLength = 2.5f;

    glm::vec3 translations[4] = {
        glm::vec3(0, 0, wallLength/2.f), // Front wall
        glm::vec3(wallLength/2.f, 0, 0), // Right wall
        glm::vec3(0, 0, -wallLength/2.f), // Back wall
        glm::vec3(-wallLength/2.f, 0, 0)  // Left wall
    };

    glm::vec4 colors[4] = {
        glm::vec4(1.f, 0.f, 0.f, 0.0f),
        glm::vec4(0.f, 1.f, 0.f, 0.0f),
        glm::vec4(0.f, 0.f, 1.f, 0.0f),
        glm::vec4(0.5f, 0.5f, 0.5f, 0.0f)
    };

    float rotations[4] = { 0, 90, 180, 270 }; // Rotations in degrees

    for (int i = 0; i < 4; ++i) {
        if (!state[i]) {
            // create wall based on state
            continue;
        }
        RenderShapeData wall;
        glm::mat4 wallCTM = glm::mat4(1.0f);

        // Rotate the wall

        // Translate the wall
        wallCTM = glm::translate(wallCTM, translations[i]);

        // Final translation to the room's position
        wallCTM = glm::translate(wallCTM, glm::vec3(x, 0, z));

        wallCTM = glm::rotate(wallCTM, glm::radians(rotations[i]), glm::vec3(0, 1, 0));


        // Scale the wall
        wallCTM = glm::scale(wallCTM, glm::vec3(wallLength, wallHeight, wallThickness));

        wall.primitive.material.cAmbient = glm::vec4(1.0f);
        wall.primitive.material.cDiffuse = colors[i];
        wall.primitive.material.cSpecular = glm::vec4(1.0f);
        wall.primitive.material.shininess = 25.f;

        wall.primitive.type = PrimitiveType::PRIMITIVE_CUBE;
        wall.ctm = wallCTM;
        walls.push_back(wall);
    }
}

void Room::generateFloor() {
    // Logic to define the floor (calculate CTM, assign materials, etc.)
    RenderShapeData floor;
    glm::mat4 floorCTM = glm::mat4(1.0f);

    // Translate the wall
    floorCTM = glm::translate(floorCTM, glm::vec3(x, -1.5f, z));

    // Scale the wall
    floorCTM = glm::scale(floorCTM, glm::vec3(5.f, 0.1f, 5.f));

    floor.primitive.material.cAmbient = glm::vec4(1.0f);
    float val = .8f;//rand() % 10 / 10.f;
    floor.primitive.material.cDiffuse = glm::vec4(val, val, val, 0.0f);
    floor.primitive.material.cSpecular = glm::vec4(1.0f);
    floor.primitive.material.shininess = 25.f;

    floor.primitive.type = PrimitiveType::PRIMITIVE_CUBE;
    floor.ctm = floorCTM;
    walls.push_back(floor);
}

void Room::generateCeiling() {
    // Logic to define the ceiling (calculate CTM, assign materials, etc.)
}

