#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class NoAlgorithmDrawer : public GraphAnimationDrawer
{
public:
    NoAlgorithmDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize) :
        GraphAnimationDrawer(graph, nodeHandlers, windowSize)
    {
    }

    virtual ~NoAlgorithmDrawer() = default;

protected:
    virtual void draw() override { GraphDrawer::draw(); }
    virtual void drawAlgorithmState() override {}
    //virtual void drawAlgorithmResult(int startNode) override { GraphDrawer::draw(); }
    virtual void prepareNewState() override {}
    virtual void prepareAnimation(int startNode) override {}
    virtual void createLegend() override {}
    virtual void animationGoToLast() override {}
private:
};
