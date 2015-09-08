#ifndef GRAPH_STUDIO_GENERATORS_H
#define GRAPH_STUDIO_GENERATORS_H
#include "Options.h"
#include <vector>

std::vector<ci::vec2> generateGridPositions(int N, int columns, int rows);
void generateGrid(const GraphParamsGrid& params, Graph &g, std::vector<ci::vec2> &nodePositions);
void generateTriangleMesh(const GraphParamsTriangleMesh& params, Graph &g, std::vector<ci::vec2> &nodePositions);

#endif // GRAPH_STUDIO_GENERATORS_H