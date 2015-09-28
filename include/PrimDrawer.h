#pragma once
#include <vector>

#include "mstPrim.h"
#include "GraphAnimationDrawer.h"

class PrimDrawer: public GraphAnimationDrawer
{
public:
    PrimDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
        GraphAnimationDrawer(graph, nodeHandlers, window)
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
    void drawNextEdges();
    std::string edgeToString(const GraphEdge& edge);

    std::vector<graph_algorithm_capture::MstPrimState> states;
};
