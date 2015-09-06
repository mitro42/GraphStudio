#include "stdafx.h"
#include "PrimDrawer.h"
#include "Options.h"

void PrimDrawer::prepareNewState()
{

}

void PrimDrawer::drawAlgorithmState()
{
    if (animationState >= int(states.size()))
        return;

    startDrawing();

    auto state = states[animationState];
    drawEdges();
    const float highlightedWidth = Options::instance().highlighedEdgeWidth;
    const ColorScheme &cs = Options::instance().currentColorScheme;
    for (const auto& e : state.mst)
    {
        drawEdge(e.from, e.to, cs.highlightedEdgeColor2, highlightedWidth);
    }

    for (const auto& e : state.edges)
    {
        drawEdge(e.from, e.to, cs.highlightedEdgeColor3, highlightedWidth);
    }

    for (const auto& inspectedEdge: state.inspectedEdges)
    {
        drawEdge(inspectedEdge.from, inspectedEdge.to, cs.highlightedEdgeColor1, highlightedWidth);
    }

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(state.visited[nodeIdx]);
    }

    drawLabels();
}

void PrimDrawer::createLegend()
{
    legend.clear();
    auto &cs = Options::instance().currentColorScheme;
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor2, "MST");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Just found");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor3, "Found, not decided");
    legend.add(LegendType::edge, cs.edgeColor, "Not processed yet");
    legend.add(LegendType::node, cs.highlightedNodeColor1, "Finished");    
    legend.add(LegendType::node, cs.nodeColor, "Remaining");
}



void PrimDrawer::prepareAnimation()
{
    GraphAnimationDrawer::prepareAnimation();
    states = graph_algorithm_capture::mstPrimCaptureStates(*g, Options::instance().startNode - 1);
    animationLastState = int(states.size());
}

void PrimDrawer::drawAlgorithmResult()
{
    startDrawing();
    drawEdges();
    auto edges = mstPrim(*g, Options::instance().startNode - 1);
    for (const auto &e : edges)
    {
        drawEdge(e.from, e.to, Options::instance().currentColorScheme.highlightedEdgeColor2, Options::instance().highlighedEdgeWidth);
    }
    drawNodes();
    drawLabels();
}