#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class NoAlgorithmDrawer : public GraphAnimationDrawer
{
public:
    NoAlgorithmDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers) : GraphAnimationDrawer(graph, nodeHandlers)
    {
    }
    ~NoAlgorithmDrawer() = default;

    virtual void draw() { GraphDrawer::draw(); }
    virtual void drawAlgorithmState() override {}
    virtual void drawAlgorithmResult() override { GraphDrawer::draw(); }
    virtual void prepareNewState() override {}
    virtual void prepareAnimation() override {}
private:
};
