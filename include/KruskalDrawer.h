#pragma once
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"

class KruskalDrawer : public GraphAnimationDrawer
{
public:
    KruskalDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
        GraphAnimationDrawer(graph, nodeHandlers, window)
    {
    }

    ~KruskalDrawer() = default;

protected:
    virtual void drawAlgorithmState() override;
    virtual void drawAlgorithmResult() override;
    virtual void prepareNewState() override;
    virtual void prepareAnimation() override;
    virtual void createLegend() override;
private:
    void drawColorScale(const std::vector<ci::Color> &colorScale);
    std::vector<ci::Color> nodeGroupColors;
    std::vector<graph_algorithm_capture::MstKruskalState> states;
};
