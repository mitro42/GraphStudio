#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class KruskalDrawer : public GraphAnimationDrawer
{
public:
    KruskalDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers) : GraphAnimationDrawer(graph, nodeHandlers)
    {
    }

    ~KruskalDrawer();

    virtual void drawAlgorithmState() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
private:
    void drawColorScale(const std::vector<ci::Color> &colorScale);
    std::vector<ci::Color> nodeGroupColors;
    std::vector<graph_algorithm_capture::MstKruskalState> states;
};
