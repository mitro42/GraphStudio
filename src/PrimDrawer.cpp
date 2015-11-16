#include "stdafx.h"
#include "PrimDrawer.h"
#include "Options.h"

const int PrimDrawer::infoPanelWidth;

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
    const float highlightedWidth = getDrawingSettings().highlightedEdgeWidth;
    const ColorScheme &cs = getColorScheme();

    auto edgeParams = createDefaultEdgeParams();

    for (const auto& e : state.mst)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor2, getDrawingSettings().highlightedEdgeWidth);
    }

    for (const auto& e : state.nonMst)
    {
        edgeParams[e].color = cs.darkEdgeColor;
    }

    for (const auto& e : state.edges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor3, getDrawingSettings().highlightedEdgeWidth);
    }

    for (const auto& e: state.inspectedEdges)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor1, getDrawingSettings().highlightedEdgeWidth);
    }

    drawEdges(edgeParams);

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(cs, state.visited[nodeIdx]);
    }
   
    drawLabels(edgeParams);
    drawStepDescription(state.description);
    drawNextEdges();
}

void PrimDrawer::createLegend()
{
    legend.clear();
    auto &cs = getColorScheme();
	legend.setBackgroundColor(cs.legendBackgroundColor);
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor2, "MST");
    legend.add(LegendType::highlightedEdge, cs.darkEdgeColor, "Not in MST");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Just found/checked");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor3, "Undecided");
}

std::string PrimDrawer::edgeToString(const GraphEdge& edge)
{
    std::stringstream ss;
    ss << std::setw(3) << std::fixed << std::setprecision(getWeightPrecision())
        << edge.weight << " (" << std::setw(1) << edge.from + 1 << " - " << std::setw(1) << edge.to + 1 << ")";
    return ss.str();
}

void PrimDrawer::drawNextEdges()
{
    if (!animationStateDescriptionVisible)
        return;

    const int lineHeight = 40;
    int nextEdgeListLength = 10;
    int baseX = int(windowSize.getWidth() - infoPanelWidth);
    int baseY = 100;
    auto &state = states[animationState];
    edgeTextureFont->drawString("Next edges", ci::vec2(baseX + 30, baseY));
    auto it = state.edges.begin();        
    int line = 0;
    if (state.description.find("Check edge") == 0)
    {
        ci::gl::color(getColorScheme().highlightedEdgeColor1);
        edgeTextureFont->drawString(edgeToString(*state.inspectedEdges[0]), ci::vec2(baseX, baseY + lineHeight));
        line++;
        nextEdgeListLength--;
    }

    for (int i = 0; i < nextEdgeListLength && it != state.edges.end(); ++i, ++it, ++line)
    {
        auto &edge = **it;
        ci::gl::color(getColorScheme().edgeColor);
        if (std::find(begin(state.inspectedEdges), end(state.inspectedEdges), *it) != state.inspectedEdges.end())
        {
            ci::gl::color(getColorScheme().highlightedEdgeColor1);
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
        drawEdge(e.from, e.to, getColorScheme().highlightedEdgeColor2, getDrawingSettings().highlightedEdgeWidth);
    }
    drawNodes();
    drawLabels();
}