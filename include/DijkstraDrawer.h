#pragma once
#include <vector>

#include "shortestPathDijkstra.h"
#include "GraphAnimationDrawer.h"

class DijkstraDrawer : public GraphAnimationDrawer
{
public:
    DijkstraDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize) :
        GraphAnimationDrawer(graph, nodeHandlers, windowSize)
    {
    }

    virtual ~DijkstraDrawer() = default;
protected:
    virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
    virtual void createLegend() override;
    virtual void animationGoToLast() override { animationState = int(states.size()) - 1; }
private:
    std::vector<graph_algorithm_capture::ShortestPathEdgeWeightDijkstraState> states;
};
