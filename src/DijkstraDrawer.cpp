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
    auto &cs = Options::instance().currentColorScheme;
    legend.add(LegendType::arrow, cs.edgeColor, "Not reached");
    legend.add(LegendType::highlightedArrow, cs.highlightedEdgeColor1, "Currently inspected");
    legend.add(LegendType::highlightedArrow, cs.highlightedEdgeColor2, "Minimal paths");
    legend.add(LegendType::arrow, cs.darkEdgeColor, "Processed and ignored");
    legend.add(LegendType::node, cs.nodeColor, "Not reached");
    legend.add(LegendType::node, cs.highlightedNodeColor1, "Currently inspected");
    legend.add(LegendType::node, cs.highlightedNodeColor2, "Reached but not processed");
    legend.add(LegendType::node, cs.highlightedNodeColor3, "Finished");
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
    const auto &cs = Options::instance().currentColorScheme;
    // Edges -----
    // preparing edge parameters before drawing
    std::map<std::shared_ptr<GraphEdge>, std::pair<ci::Color, float>> edgeParams;
    // by default edges are drawn with edgeColor and edgeWidth (no highlighting)
    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        auto &node = g->getNode(nodeIdx);
        for (auto edgePtr : node)
        {
            edgeParams[edgePtr] = std::make_pair(cs.edgeColor, Options::instance().edgeWidth);
        }
    }

    // now the params for all dropped edges are overwritten
    for (auto &edgePtr : state.processedEdges)
    {
        edgeParams[edgePtr] = std::make_pair(cs.darkEdgeColor, Options::instance().edgeWidth);
    }

    // overwriting the edges of the current best paths
    for (auto &p : state.path)
    {
        if (p.second == nullptr)
            continue;
        edgeParams[p.second] = std::make_pair(cs.highlightedEdgeColor2, Options::instance().highlighedEdgeWidth);
    }

    // and finally the currently inspected edge
    if (state.inspectedEdge != nullptr)
        edgeParams[state.inspectedEdge] = std::make_pair(cs.highlightedEdgeColor1, Options::instance().highlighedEdgeWidth);

    for (auto &p : edgeParams)
    {
        drawEdge(p.first->from, p.first->to, p.second.first, p.second.second);
    }

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

    // Labels -----
    // preparing edge label colors
    std::map<std::shared_ptr<GraphEdge>, ci::ColorA> edgeLabelColors;    
    for (auto &edgePtr : edgeParams)
    {
        edgeLabelColors[edgePtr.first] = edgePtr.second.first;
    }
    drawLabels(edgeLabelColors);
    
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
        drawEdge(edgePtr->from, edgePtr->to, Options::instance().currentColorScheme.highlightedEdgeColor2, Options::instance().highlighedEdgeWidth);
    }
    drawNodes();
    drawLabels();
    */
}