#include "stdafx.h"
#include "PrimDrawer.h"
#include "Options.h"

void PrimDrawer::prepareNewState()
{

}


void PrimDrawer::drawAlgorithmState()
{
    setChanged();
    startDrawing();
    if (animationState >= int(states.size()))
        return;

    if (animationState == -1)
    {
        if (g->getNodeCount() == 0)
            return;

        prepareAnimation();
    }


    auto state = states[animationState];
    const float highlightedWidth = Options::instance().highlighedEdgeWidth;
    const ColorScheme &cs = Options::instance().currentColorScheme;

    auto edgeParams = createDefaultEdgeParams();

    for (const auto& e : state.mst)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor2, Options::instance().highlighedEdgeWidth);
    }

    for (const auto& e : state.nonMst)
    {
        edgeParams[e].color = cs.darkEdgeColor;
    }

    for (const auto& e : state.edges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor3, Options::instance().highlighedEdgeWidth);
    }

    for (const auto& e: state.inspectedEdges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor1, Options::instance().highlighedEdgeWidth);
    }

    drawEdges(edgeParams);

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(state.visited[nodeIdx]);
    }
   
    drawLabels(edgeParams);

    drawStepDescription(state.description);
}

void PrimDrawer::createLegend()
{
    legend.clear();
    auto &cs = Options::instance().currentColorScheme;
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor2, "MST");
    legend.add(LegendType::highlightedEdge, cs.darkEdgeColor, "Not in MST");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Just found");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor3, "Found, not decided");
    legend.add(LegendType::node, cs.highlightedNodeColor1, "Finished");    

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