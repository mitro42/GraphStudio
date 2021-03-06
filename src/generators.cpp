#include "stdafx.h"
#include "Graph.h"
#include "generators.h"
#include "Options.h"

#include <cinder/Rand.h>

std::vector<ci::vec2> generateGridPositions(int N, int columns, int rows)
{
    std::vector<ci::vec2> nodePositions(N);
    int row = 0;
    int col = 0;
    int margin = 100;
    int width = 800; //  window->getWidth() - (2 * margin); // TODO: replace with window size independent coordinates
    int height = 600; // window->getHeight() - (2 * margin);
    float xStep = float(width) / (columns - 1);
    float yStep = float(height) / (rows - 1);

    for (int i = 0; i < N; ++i)
    {
        nodePositions[i] = ci::vec2(margin + col * xStep, margin + row*yStep);
        col++;
        if (col == columns)
        {
            col = 0;
            row++;
        }
    }
    return nodePositions;
}



void GraphGeneratorGrid::run(Graph &g, std::vector<ci::vec2> &nodePositions) const
{
    g.clear(directed);
    g.setWeightedEdges(true);
    g.setWeightedNodes(false);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            g.addNode();
        }
    }

    if (horizontal)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns - 1; j++)
            {
                int start = i*columns + j;
                g.addEdge(start, start + 1, 1.0);
            }
        }
    }

    if (vertical)
    {
        for (int i = 0; i < rows - 1; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                int start = i*columns + j;
                g.addEdge(start, start + columns, 1.0);
            }
        }
    }

    if (upDiagonal)
    {
        for (int i = 1; i < rows; i++)
        {
            for (int j = 0; j < columns - 1; j++)
            {
                int start = i*columns + j;
                g.addEdge(start, start + 1 - columns, 1.0);
            }
        }
    }

    if (downDiagonal)
    {
        for (int i = 0; i < rows - 1; i++)
        {
            for (int j = 0; j < columns - 1; j++)
            {
                int start = i*columns + j;
                g.addEdge(start, start + 1 + columns);
            }
        }
    }

    nodePositions = generateGridPositions(rows * columns, columns, rows);
}


void GraphGeneratorTriangleMesh::run(Graph &g, std::vector<ci::vec2> &nodePositions) const
{
    g.clear(directed);
    struct Edge{
        int from = 0;
        int to = 0;
        bool right = true;
        Edge() = default;
        Edge(int from, int to, bool right) : from(from), to(to), right(right) {}
    };

    std::vector<Edge> outerEdges;
    ci::randSeed(42);
    g.setWeightedEdges(true);
    g.setWeightedNodes(false);
    g.addNode();
    g.addNode();
    g.addEdge(0, 1);
    outerEdges.emplace_back(0, 1, true);
    ci::vec2 center(400, 300); // TODO: replace with window size independent coordinates
    ci::vec2 centerOffset = ci::Rand::randVec2() * 25.0f;
    nodePositions.push_back(center + centerOffset);
    nodePositions.push_back(center - centerOffset);

    // TODO find out why triangles without this-> won't compile with VS
    while (nodePositions.size() < this->triangles+2)
    {
        // select a random edge on perimeter of the mesh
        int edgeIdx = ci::randInt(int32_t(outerEdges.size()));
        auto edge = outerEdges[edgeIdx];

        // remove from the outerEdges vector
        size_t size = outerEdges.size();
        std::swap(outerEdges[edgeIdx], outerEdges[size - 1]);
        outerEdges.resize(size - 1);

        int newIdx = g.addNode();
        g.addEdge(newIdx, edge.from, 1.0);
        g.addEdge(newIdx, edge.to, 1.0);
        outerEdges.emplace_back(newIdx, edge.from, false);
        outerEdges.emplace_back(newIdx, edge.to, false);


        ci::vec2 offset = glm::normalize(nodePositions[edge.from] - nodePositions[edge.to]);        
        ci::vec2 newPos = (nodePositions[edge.from] + nodePositions[edge.to]) / 2.0f;
        newPos += glm::rotate(offset, glm::half_pi<float>()) * sqrt(3.0f) * 50.0f;
        nodePositions.push_back(newPos);
    }
}