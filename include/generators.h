#ifndef GRAPH_STUDIO_GENERATORS_H
#define GRAPH_STUDIO_GENERATORS_H
#include "Options.h"
#include <vector>

std::vector<ci::Vec2f> generateGridPositions(int N, int columns, int rows);
void generateGrid(const GraphParamsGrid& params, Graph &g, std::vector<ci::Vec2f> &nodePositions);
void generateTriangleMesh(const GraphParamsTriangleMesh& params, Graph &g, std::vector<ci::Vec2f> &nodePositions);

#endif // GRAPH_STUDIO_GENERATORS_H