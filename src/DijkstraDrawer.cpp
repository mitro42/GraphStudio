#include "stdafx.h"
#include "DijkstraDrawer.h"
#include "Options.h"

void DijkstraDrawer::prepareNewState()
{
    auto state = states[animationState];
    std::string text;
    for (size_t i = 0; i < state.path.size(); ++i)
    {
        if (state.path[i].second == -1)
            continue;
        std::string line = std::to_string(state.path[i].second + 1) + "->" +
            std::to_string(i + 1) +
            "(" + std::to_string(int(state.path[i].first)) + ")";

        if (!text.empty())
        {
            text += "\n";
        }
        text += line;
    }
    /*
    ci::TextBox legendTextBox = ci::TextBox().alignment(ci::TextBox::RIGHT).font(legendFont).size(ci::Vec2i(200, 200));
    legendTextBox.setColor(ci::Color(1.0f, 1.0f, 1.0f));
    legendTextBox.setBackgroundColor(ci::ColorA(0, 0, 0, 0.5));
    legendTextBox.text(text);    
    */    
}

void DijkstraDrawer::createLegend()
{
    legend.clear();
    auto &cs = Options::instance().currentColorScheme;
    legend.add(LegendType::highlightedArrow, cs.highlightedEdgeColor2, "Minimal paths");
    legend.add(LegendType::highlightedArrow, cs.highlightedEdgeColor1, "Inspected");
    legend.add(LegendType::arrow, cs.edgeColor, "Not processed yet");
    legend.add(LegendType::node, cs.highlightedNodeColor1, "Finished");
    legend.add(LegendType::node, cs.highlightedNodeColor2, "Reached");
    legend.add(LegendType::node, cs.nodeColor, "Not reached yet");
}

void DijkstraDrawer::prepareAnimation()
{
    GraphAnimationDrawer::prepareAnimation();
    states = graph_algorithm_capture::edgeWeightDijkstraCaptureStates(*g, Options::instance().startNode - 1, -1);
    animationLastState = states.size();
}


void DijkstraDrawer::drawAlgorithmState()
{
    if (animationState >= int(states.size()))
        return;
    startDrawing();

    auto state = states[animationState];

    const auto &cs = Options::instance().currentColorScheme;
    drawEdges();
    for (int i = 0; i < int(state.path.size()); ++i)
    {
        auto from = state.path[i].second;
        if (from == -1)
            continue;
        if (from == i)
            continue;
        drawEdge(from, i, cs.highlightedEdgeColor2, Options::instance().highlighedEdgeWidth);
    }

    if (state.inspectedEdge.from != -1)
    {
        drawEdge(state.inspectedEdge.from, state.inspectedEdge.to, cs.highlightedEdgeColor1, Options::instance().highlighedEdgeWidth);
    }

    std::vector<ci::Color> nodeHighlight(g->getNodeCount(), cs.nodeColor);
    for (auto &p : state.openNodes)
    {
        nodeHighlight[p.second] = cs.highlightedNodeColor2;
    }

    if (state.inspectedNode != -1)
    {
        nodeHighlight[state.inspectedNode] = cs.highlightedNodeColor1;
    }

    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers[i]->draw(nodeHighlight[i]);
    }

    drawLabels();
}


void DijkstraDrawer::drawAlgorithmResult()
{
    startDrawing();
    drawEdges();
    auto tree = edgeWeightDijkstra(*g, Options::instance().startNode - 1, -1);
    for (int i = 0; i < int(tree.size()); ++i)
    {
        auto from = tree[i].second;
        if (from == -1)
            continue;
        if (from == i)
            continue;
        drawEdge(from, i, true);
    }
    drawNodes();
    drawLabels();
}