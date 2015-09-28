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


    auto &state = states[animationState];
    const float highlightedWidth = Options::instance().highlightedEdgeWidth;
    const ColorScheme &cs = Options::instance().currentColorScheme;

    auto edgeParams = createDefaultEdgeParams();

    for (const auto& e : state.mst)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor2, Options::instance().highlightedEdgeWidth);
    }

    for (const auto& e : state.nonMst)
    {
        edgeParams[e].color = cs.darkEdgeColor;
    }

    for (const auto& e : state.edges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor3, Options::instance().highlightedEdgeWidth);
    }

    for (const auto& e: state.inspectedEdges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor1, Options::instance().highlightedEdgeWidth);
    }

    drawEdges(edgeParams);

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(state.visited[nodeIdx]);
    }
   
    drawLabels(edgeParams);
    drawStepDescription(state.description);
    drawNextEdges();
}

void PrimDrawer::createLegend()
{
    Options::instance().infoPanelWidth = 300;
    legend.clear();
    auto &cs = Options::instance().currentColorScheme;
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor2, "MST");
    legend.add(LegendType::highlightedEdge, cs.darkEdgeColor, "Not in MST");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Just found/checked");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor3, "Undecided");
}

std::string PrimDrawer::edgeToString(const GraphEdge& edge)
{
    std::stringstream ss;
    ss << std::setw(3) << std::fixed << std::setprecision(Options::instance().weightPrecision)
        << edge.weight << " (" << std::setw(1) << edge.from + 1 << " - " << std::setw(1) << edge.to + 1 << ")";
    return ss.str();
}

void PrimDrawer::drawNextEdges()
{
    if (!animationStateDescriptionVisible)
        return;

    const int lineHeight = 40;
    int nextEdgeListLength = 10;
    int baseX = int(window->getWidth() - Options::instance().infoPanelWidth);
    int baseY = 100;
    auto &state = states[animationState];
    edgeTextureFont->drawString("Next edges", ci::vec2(baseX + 30, baseY));
    auto it = state.edges.begin();        
    int line = 0;
    if (state.description.find("Check edge") == 0)
    {
        ci::gl::color(Options::instance().currentColorScheme.highlightedEdgeColor1);
        edgeTextureFont->drawString(edgeToString(*state.inspectedEdges[0]), ci::vec2(baseX, baseY + lineHeight));
        line++;
        nextEdgeListLength--;
    }

    for (int i = 0; i < nextEdgeListLength && it != state.edges.end(); ++i, ++it, ++line)
    {
        auto &edge = **it;
        ci::gl::color(Options::instance().currentColorScheme.edgeColor);
        if (std::find(begin(state.inspectedEdges), end(state.inspectedEdges), *it) != state.inspectedEdges.end())
        {
            ci::gl::color(Options::instance().currentColorScheme.highlightedEdgeColor1);
        }
        
        edgeTextureFont->drawString(edgeToString(edge), ci::vec2(baseX, baseY + (line + 1) * lineHeight));
    }
    if (it != state.edges.end())
    {
        edgeTextureFont->drawString("     ...", ci::vec2(baseX, baseY + (line + 1) * lineHeight));
    }
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
        drawEdge(e.from, e.to, Options::instance().currentColorScheme.highlightedEdgeColor2, Options::instance().highlightedEdgeWidth);
    }
    drawNodes();
    drawLabels();
}