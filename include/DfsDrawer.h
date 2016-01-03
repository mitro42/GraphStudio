#pragma once
#include <vector>

#include "depthFirstSearch.h"
#include "GraphAnimationDrawer.h"

class DfsDrawer: public GraphAnimationDrawer
{
public:
	DfsDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize) :
        GraphAnimationDrawer(graph, nodeHandlers, windowSize, infoPanelWidth)
    {
    }

    virtual ~DfsDrawer() = default;
protected:
	virtual void drawAlgorithmState() override;
    //virtual void drawAlgorithmResult(int startNode) override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation(int startNode) override;
    virtual void createLegend() override;
    virtual void animationGoToLast() override { animationState = int(states.size()) - 1; }
private:
	static const int infoPanelWidth = 300;
	std::vector<ci::Color> nodeOrderColors;
    std::vector<graph_algorithm_capture::DepthFirstSearchState> states;
};
