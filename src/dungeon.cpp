#include "dungeon.h"
#include "room.h"
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

// rooms as vector<Room>

Dungeon::Dungeon() {
    // initialize dungeon
    srand(time(NULL));
}

void Dungeon::generateRooms(int maxWidth, int maxHeight, int size) {
    generatePlan(maxWidth, maxHeight, size);
    //for (int i = 0; i < count; ++i) {
    //    generateRoom();
    //}
    generateWalls();

    for (const auto& row : rooms2D) {
        for (const auto& room : row) {
            rooms1D.push_back(room);
        }
    }
}

void Dungeon::destroyRooms() {
    rooms2D.clear();
    rooms1D.clear();
}

float heuristic(const glm::vec2& a, const glm::vec2& b) {
    return glm::distance(a, b);
}

struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
template <>
struct hash<Point> {
    size_t operator()(const Point& p) const {
        return hash<int>()(p.x) ^ hash<int>()(p.y);
    }
};
}

struct Node {
    Point point;
    int cost;
    int heuristic;
    Node* parent;

    Node(Point point, int cost, int heuristic, Node* parent)
        : point(point), cost(cost), heuristic(heuristic), parent(parent) {}

    int getScore() const {
        return cost + heuristic;
    }
};

struct CompareNode {
    bool operator()(const Node* lhs, const Node* rhs) const {
        return lhs->getScore() > rhs->getScore();
    }
};

int heuristic(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

std::vector<glm::vec2> reconstructPath(Node* current) {
    std::vector<glm::vec2> path;
    int timeout = 0;
    while (current != nullptr && timeout < 30) {
        timeout++;
        glm::vec2 vertex = glm::vec2(current->point.x, current->point.y);
        path.push_back(vertex);
        current = current->parent;
    }
    return path;
}

std::vector<glm::vec2> aStarSearch(std::vector<std::vector<Room>>& grid, glm::vec2 tStart, glm::vec2 tGoal) {
    std::priority_queue<Node*, std::vector<Node*>, CompareNode> openSet;
    std::unordered_map<Point, int> costSoFar;
    std::unordered_map<Point, Node*> cameFrom;

    Point start(tStart.x, tStart.y);
    Point goal(tGoal.x, tGoal.y);

    Node* startNode = new Node(start, 0, heuristic(start, goal), nullptr);
    openSet.push(startNode);
    costSoFar[start] = 0;

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        if (current->point == goal) {
            std::vector<glm::vec2> path = reconstructPath(current);
            //delete current;
            //for (auto& pair : cameFrom) {
            //    delete pair.second;
            //}
            return path;
        }

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                // limit movement to non-diagonal neighbors to keep hallways continuous/traversable
                if (dx != 0 && dy != 0) continue;
                Point neighbor(current->point.x + dx, current->point.y + dy);

                if (neighbor.x >= 0 && neighbor.y >= 0 && neighbor.x < grid.size() && neighbor.y < grid[0].size()/* && grid[neighbor.x][neighbor.y].isEmpty()*/) {
                    int newCost = costSoFar[current->point] + 1;
                    if (costSoFar.find(neighbor) == costSoFar.end() || newCost < costSoFar[neighbor]) {
                        costSoFar[neighbor] = newCost;
                        int priority = newCost + heuristic(neighbor, goal);
                        Node* nextNode = new Node(neighbor, newCost, priority, current);
                        openSet.push(nextNode);
                        cameFrom[neighbor] = nextNode;
                    }
                }
            }
        }

        //if (cameFrom.find(current->point) == cameFrom.end()) {
        //    delete current;
        //}
    }

    //for (auto& pair : cameFrom) {
    //    delete pair.second;
    //}

    return std::vector<glm::vec2>();
}

void Dungeon::generatePlan(int width, int length, int size) {
    rooms2D.resize(length);
    for (int i = 0; i < length; ++i) {
        rooms2D[i].resize(width, Room(RoomType::Empty, 0, 0)); // Initialize each room
    }


    // dungeonmasters algorithm
    // 1: place rooms. Cannot be adjacent.
    // 2: find minimum spanning tree
    // 3: choose random edges (edgeKeepChance)
    // 4: pathfind hallways with A*

    // generate random names
    // lighting
    // post processing

    int roomCount = size;
    std::vector<glm::vec2> vertices;
    // 1
    for (int i = 0; i < roomCount; ++i) {
        int x = rand() % width;
        int z = rand() % length;
        int roomWidth = std::min(5, (rand() % width)/3 + 3);
        int roomLength = std::min(5, (rand() % length)/3 + 2);
        vertices.push_back(glm::vec2(x, z));

        for (int w = 0; w < roomWidth; ++w) {
            for (int l = 0; l < roomLength; ++l) {
                int xw = x + w;
                int zl = z + l;
                if (xw < 0 || xw >= width || zl < 0 || zl >= length) {
                    continue;
                }
                rooms2D[xw][zl] = Room(RoomType::Default, xw * 5, zl * 5);
            }
        }
    }

    struct Hall {
        glm::vec2 room1;
        glm::vec2 room2;
        float weight;

        Hall(glm::vec2 room1, glm::vec2 room2, float weight) : room1(room1), room2(room2), weight(weight) {}
    };
    // 2: Placeholder for now, creates hallway between all rooms
    std::vector<Hall> halls;

    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            glm::vec2 room1 = vertices[i];
            glm::vec2 room2 = vertices[j];
            halls.emplace_back(room1, room2, std::numeric_limits<float>::max());
        }
    }


    // 3: Prim's algorithm for minimum spanning tree
    std::vector<Hall> primHalls;
    size_t numVertices = vertices.size();
    std::vector<float> key(numVertices, std::numeric_limits<float>::max());
    std::vector<bool> inMST(numVertices, false);
    std::vector<size_t> parent(numVertices, -1);

    key[0] = 0; // Start from the first vertex

    for (size_t count = 0; count < numVertices - 1; ++count) {
        // Find the minimum key vertex from the set of vertices not yet in MST
        float min = std::numeric_limits<float>::max();
        size_t minIndex;

        for (size_t v = 0; v < numVertices; ++v)
            if (!inMST[v] && key[v] < min)
                min = key[v], minIndex = v;

        // Add the chosen vertex to the MST set
        inMST[minIndex] = true;

        // Update key value and parent index of the adjacent vertices
        for (size_t v = 0; v < numVertices; ++v) {
            float weight = glm::distance(vertices[minIndex], vertices[v]);

            if (!inMST[v] && weight < key[v]) {
                parent[v] = minIndex, key[v] = weight;
            }
        }
    }
    // Construct the minimum spanning tree
    for (size_t i = 1; i < numVertices; ++i) {
        primHalls.emplace_back(vertices[parent[i]], vertices[i], key[i]);
    }

    // 4: randomly re-include hallways with a 12.5% chance
    // not yet implemented

    // 5: pathfind hallways with A*
    for (Hall hall : primHalls) {
        std::vector<glm::vec2> path = aStarSearch(rooms2D, hall.room1, hall.room2);

        for (glm::vec2 coords : path) {
            rooms2D[coords.x][coords.y] = Room(RoomType::Default, coords.x * 5, coords.y * 5);
        }
    }

    /*
    // drunken walk algorithm for testing

    int created = 0;
    int x = width/2;//rand() % width;  // Random starting X coordinate
    int z = height/2;//rand() % height; // Random starting Y coordinate

    while (created < size) {
        // Mark the current position as a room
        rooms2D[x][z] = Room(RoomType::Default, x * 5, z * 5);

        while (!rooms2D[x][z].isEmpty()) {
            // Randomly move to adjacent cells until not empty
            int dx = 0;
            int dz = 0;
            if (rand() % 2) {
                dx = rand() % 3 - 1; // -1, 0, or 1
            } else {
                dz = rand() % 3 - 1; // -1, 0, or 1
            }

            x = std::max(0, std::min(x + dx, width - 1));
            z = std::max(0, std::min(z + dz, height - 1));
        }

        created++;
    }*/
}

std::vector<Room> Dungeon::getRooms() {
    return rooms1D;
}

void Dungeon::generateWalls() {
    for (int i = 0; i < rooms2D.size(); ++i) {
        for (int j = 0; j < rooms2D[i].size(); ++j) {
            if (rooms2D[i][j].isEmpty()) {
                continue; // Skip the current room if it's empty
            }

            // front, right, back, left
            // 1 = generate wall, 0 = don't
            glm::vec4 state = glm::vec4(0);

            // generate walls
            // Right neighbor
            state[0] = (j >= rooms2D[i].size() - 1 || rooms2D[i][j+1].isEmpty());

            // Bottom neighbor
            state[1] = (i >= rooms2D.size() - 1 || rooms2D[i+1][j].isEmpty());

            // Left neighbor
            state[2] = (j <= 0 || rooms2D[i][j-1].isEmpty());

            // Top neighbor
            state[3] = (i <= 0 || rooms2D[i-1][j].isEmpty());

            rooms2D[i][j].generateWalls(state);
        }
    }
}


