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
protected:
    virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
    virtual void createLegend() override;
private:
    std::vector<graph_algorithm_capture::MstPrimState> states;
};
