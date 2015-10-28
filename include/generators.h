#ifndef GRAPH_STUDIO_GENERATORS_H
#define GRAPH_STUDIO_GENERATORS_H
#include "Options.h"
#include <vector>

struct GraphGenerator
{
    virtual void run(Graph &g, std::vector<ci::vec2> &nodePositions) const = 0;
};

struct GraphGeneratorGrid : public GraphGenerator
{
    int columns = 3;
    int rows = 3;
    bool directed = false;
    bool horizontal = true;
    bool vertical = true;
    bool upDiagonal = true;
    bool downDiagonal = true;

    virtual void run(Graph &g, std::vector<ci::vec2> &nodePositions) const;
};


struct GraphGeneratorTriangleMesh : public GraphGenerator
{
    int triangles = 10;
    float randomness = 0.0f;
    bool directed = false;
    virtual void run(Graph &g, std::vector<ci::vec2> &nodePositions) const;
};


std::vector<ci::vec2> generateGridPositions(int N, int columns, int rows);

#endif // GRAPH_STUDIO_GENERATORS_H