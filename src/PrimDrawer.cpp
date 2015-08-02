#include "PrimDrawer.h"
#include "Options.h"

void PrimDrawer::prepareNewState()
{

}

void PrimDrawer::drawAlgorithmState()
{
    if (animationState >= int(states.size()))
        return;

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


    drawEdge(state.inspectedEdge.from, state.inspectedEdge.to, cs.highlightedEdgeColor1, highlightedWidth);

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(state.visited[nodeIdx]);
    }

    drawLabels();
}

void PrimDrawer::prepareAnimation()
{
    GraphAnimationDrawer::prepareAnimation();
    states = graph_algorithm_capture::mstPrimCaptureStates(*g, Options::instance().startNode - 1);
    animationLastState = states.size();
}

void PrimDrawer::drawAlgorithmResult()
{
    startDrawing();
    drawEdges();
    auto edges = mstPrim(*g, Options::instance().startNode - 1);
    for (const auto &e : edges)
    {
        drawEdge(e.from, e.to, true);
    }
    drawNodes();
    drawLabels();
}