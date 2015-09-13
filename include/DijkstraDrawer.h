#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class DijkstraDrawer : public GraphAnimationDrawer
{
public:
    DijkstraDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
        GraphAnimationDrawer(graph, nodeHandlers, window)
    {
    }

    virtual ~DijkstraDrawer() = default;
protected:
    virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
    virtual void createLegend() override;
    virtual void animationGoToLast() override { animationState = states.size() - 1; }
private:
    std::vector<graph_algorithm_capture::ShortestPathEdgeWeightDijkstraState> states;
};
