#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class DijkstraDrawer : public GraphAnimationDrawer
{
public:
    DijkstraDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers) : GraphAnimationDrawer(graph, nodeHandlers)
    {
        legendFont = ci::Font("InputMono Black", 15);
        legendTextureFont = ci::gl::TextureFont::create(legendFont);
    }

    ~DijkstraDrawer();

    virtual void drawAlgorithmState() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
private:
    std::vector<graph_algorithm_capture::ShortestPathEdgeWeightDijkstraState> states;
};