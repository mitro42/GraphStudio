#include "stdafx.h"
#include "DijkstraDrawer.h"
#include "Options.h"

#include <algorithm>

void DijkstraDrawer::prepareNewState()
{
    auto state = states[animationState];
    setChanged();
}

void DijkstraDrawer::createLegend()
{
    legend.clear();
	legend.setBackgroundColor(getColorScheme().legendBackgroundColor);
    legend.add(LegendType::arrow, getColorScheme().edgeColor, "Not reached");
    legend.add(LegendType::highlightedArrow, getColorScheme().highlightedEdgeColor1, "Currently inspected");
    legend.add(LegendType::highlightedArrow, getColorScheme().highlightedEdgeColor2, "Minimal paths");
    legend.add(LegendType::arrow, getColorScheme().darkEdgeColor, "Processed and ignored");
    legend.add(LegendType::node, getColorScheme().nodeColor, "Not reached");
    legend.add(LegendType::node, getColorScheme().highlightedNodeColor1, "Currently inspected");
    legend.add(LegendType::node, getColorScheme().highlightedNodeColor2, "Reached but not processed");
    legend.add(LegendType::node, getColorScheme().highlightedNodeColor3, "Finished");
}

void DijkstraDrawer::prepareAnimation()
{
    states = graph_algorithm_capture::edgeWeightDijkstraCaptureStates(*g, Options::instance().startNode - 1, -1);
    animationLastState = int(states.size());
    GraphAnimationDrawer::prepareAnimation();
    
}


void DijkstraDrawer::drawAlgorithmState()
{
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
	const auto &cs = getColorScheme();
    // Edges -----
    // preparing edge parameters before drawing
    auto edgeParams = createDefaultEdgeParams();
    
    // now the params for all dropped edges are overwritten
    for (auto &edgePtr : state.processedEdges)
    {
        edgeParams[edgePtr].color = cs.darkEdgeColor;
    }

    // overwriting the edges of the current best paths
    for (auto &p : state.path)
    {
        if (p.second == nullptr)
            continue;
        edgeParams[p.second] = EdgeDrawParams(cs.highlightedEdgeColor2, getDrawingSettings().highlightedEdgeWidth);
    }

    // and finally the currently inspected edge
    if (state.inspectedEdge != nullptr)
        edgeParams[state.inspectedEdge] = EdgeDrawParams(cs.highlightedEdgeColor1, getDrawingSettings().highlightedEdgeWidth);

    drawEdges(edgeParams);
    

    // Nodes ---------
    // Preparing open nodes
    std::vector<ci::Color> nodeHighlight(g->getNodeCount(), cs.nodeColor);
    for (auto &p : state.openNodes)
    {
        nodeHighlight[p.second] = cs.highlightedNodeColor2;
    }

    // Preparing closed nodes
    for (auto &p : state.closedNodes)
    {
        nodeHighlight[p.second] = cs.highlightedNodeColor3;
    }

    // Preparing current node
    if (state.inspectedNode != -1)
    {
        nodeHighlight[state.inspectedNode] = cs.highlightedNodeColor1;
    }

    // Drawing nodes
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers[i]->draw(nodeHighlight[i]);
    }

    drawLabels(edgeParams);    
    drawStepDescription(state.description);
}


void DijkstraDrawer::drawAlgorithmResult()
{    
    if (states.empty())
    {
        prepareAnimation();
    }

    animationState = int(states.size() - 1);
    drawAlgorithmState();
    /*
    startDrawing();
    drawEdges();
    auto tree = edgeWeightDijkstra(*g, Options::instance().startNode - 1, -1);
    for (int i = 0; i < int(tree.size()); ++i)
    {
        auto edgePtr = tree[i].second;
        if (edgePtr == nullptr)
            continue;
        drawEdge(edgePtr->from, edgePtr->to, Options::instance().currentColorScheme.highlightedEdgeColor2, Options::instance().highlightedEdgeWidth);
    }
    drawNodes();
    drawLabels();
    */
}