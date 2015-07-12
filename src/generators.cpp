#include "Graph.h"
#include "generators.h"
#include "Options.h"

#include <cinder/Vector.h>

std::vector<ci::Vec2f> generateGridPositions(int N, int columns, int rows)
{
    std::vector<ci::Vec2f> nodePositions(N);
    //std::unique_lock<std::recursive_mutex> guard(updateMutex);
    int row = 0;
    int col = 0;
    int margin = 100;
    int width = 800; //  window->getWidth() - (2 * margin); // TODO: replace with window size independent coordinates
    int height = 600; // window->getHeight() - (2 * margin);
    float xStep = float(width) / (columns - 1);
    float yStep = float(height) / (rows - 1);

    for (int i = 0; i < N; ++i)
    {
        nodePositions[i] = ci::Vec2f(margin + col * xStep, margin + row*yStep);
        col++;
        if (col == columns)
        {
            col = 0;
            row++;
        }
    }
    return nodePositions;
}



void generateGrid(const GraphParamsGrid& params, Graph &g, std::vector<ci::Vec2f> &nodePositions)
{

    g.clear();    
    g.setDirected(params.directed);
    g.setWeightedEdges(false);
    g.setWeightedNodes(false);
    for (int i = 0; i < params.rows; i++)
    {
        for (int j = 0; j < params.columns; j++)
        {
            g.addNode();
        }
    }

    if (params.horizontal)
    {
        for (int i = 0; i < params.rows; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1);
            }
        }
    }

    if (params.vertical)
    {
        for (int i = 0; i < params.rows - 1; i++)
        {
            for (int j = 0; j < params.columns; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + params.columns);
            }
        }
    }

    if (params.upDiagonal)
    {
        for (int i = 1; i < params.rows; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1 - params.columns);
            }
        }
    }

    if (params.downDiagonal)
    {
        for (int i = 0; i < params.rows - 1; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1 + params.columns);
            }
        }
    }

    nodePositions = generateGridPositions(params.rows * params.columns, params.columns, params.rows);
}


void generateTriangleMesh(const GraphParamsTriangleMesh& params, Graph &g, std::vector<ci::Vec2f> &nodePositions)
{
    /*
    struct Edge{
        int from = 0;
        int to = 0;
        bool right = true;
        Edge() = default;
        Edge(int from, int to, bool right) : from(from), to(to), right(right) {}
    };

    //std::vector<Edge> outerEdges;
    ci::randSeed(42);
    g.setDirected(false);
    g.setWeightedEdges(false);
    g.setWeightedNodes(false);
    g.clear();
    g.addNode();
    g.addNode();
    g.addEdge(0, 1);
    outerEdges.emplace_back(0, 1, true);
    ci::Vec2f center(window->getWidth() / 2, window->getHeight() / 2);
    ci::Vec2f centerOffset = ci::randVec2f() * 25;
    nodePositions.push_back(center + centerOffset);
    nodePositions.push_back(center - centerOffset);
    while (nodePositions.size() < GraphParamsTriangleMesh::instance().triangles)
    {
        int edgeIdx = ci::randInt(outerEdges.size());
        auto edge = outerEdges[edgeIdx];
        int newIdx = g.addNode();
        g.addEdge(newIdx, edge.from);
        g.addEdge(newIdx, edge.to);
        outerEdges.emplace_back(newIdx, edge.from, false);
        outerEdges.emplace_back(newIdx, edge.to, false);
        size_t size = outerEdges.size();
        std::swap(outerEdges[edgeIdx], outerEdges[size - 1]);
        outerEdges.resize(size - 1);
        ci::Vec2f offset = (nodePositions[edge.from] - nodePositions[edge.to]).normalized();
        offset.rotate(M_PI / 2);
        ci::Vec2f newPos = (nodePositions[edge.from] + nodePositions[edge.to]) / 2;
        newPos += offset * sqrt(3) * 50;
        nodePositions.push_back(newPos);
    }
    */
}