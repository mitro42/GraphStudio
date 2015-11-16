#pragma once
#include <vector>

#include "mstPrim.h"
#include "GraphAnimationDrawer.h"

class PrimDrawer: public GraphAnimationDrawer
{
public:
    PrimDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize) :
        GraphAnimationDrawer(graph, nodeHandlers, windowSize, infoPanelWidth)
    {
    }

    virtual ~PrimDrawer() = default;
protected:
	virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
    virtual void createLegend() override;
    virtual void animationGoToLast() override { animationState = int(states.size()) - 1; }
private:
	static const int infoPanelWidth = 300;
    void drawNextEdges();
    std::string edgeToString(const GraphEdge& edge);

    std::vector<graph_algorithm_capture::MstPrimState> states;
};
