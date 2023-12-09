#ifndef ROOM_H
#define ROOM_H

#include "utils/sceneparser.h"
#include <vector>

enum class RoomType {
    Default,
    Empty, // Used for uninitialized or empty spaces
    // Add other room types as needed
};

class Room {
public:
    RenderShapeData floor;
    RenderShapeData ceiling;
    std::vector<RenderShapeData> walls;

    bool isEmpty();
    void generateWalls(glm::vec4 state);

    Room(RoomType room, float x, float y); // Constructor

private:
    void generateFloor();
    void generateCeiling();

    float x;
    float z;
};

#endif
