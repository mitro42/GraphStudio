#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class PrimDrawer: public GraphAnimationDrawer
{
public:
    PrimDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
        GraphAnimationDrawer(graph, nodeHandlers, window)
    {
    }

    ~PrimDrawer() = default;

    virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
private:
    std::vector<graph_algorithm_capture::MstPrimState> states;
};
