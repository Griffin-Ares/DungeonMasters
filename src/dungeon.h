#ifndef DUNGEON_H
#define DUNGEON_H

#include "room.h"
#include <vector>

class Dungeon
{
private:
    //std::vector<Room> rooms;

public:
    Dungeon();

    // can generate rooms with generateRooms(count)
    void generateRooms(int maxWidth, int maxHeight, int count);
    void destroyRooms();

    // rooms can then be indexed with getRoom(index)
    std::vector<Room> getRooms();

    void generateWalls();

    void generatePlan(int width, int height, int size);

    std::vector<std::vector<Room>> rooms2D;

    std::vector<Room> rooms1D;
};

#endif // DUNGEON_H
