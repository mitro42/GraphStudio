#include "stdafx.h"
#include "KruskalDrawer.h"
#include "Options.h"

void KruskalDrawer::prepareNewState()
{
}


std::string KruskalDrawer::edgeToString(const GraphEdge& edge) const
{
    std::stringstream ss;
    ss << std::setw(3) << std::fixed << std::setprecision(Options::instance().weightPrecision)
        << edge.weight << " (" << std::setw(1) << edge.from + 1 << " - " << std::setw(1) << edge.to + 1 << ")";
    return ss.str();
}



void KruskalDrawer::drawNextEdges() const
{
    if (!animationStateDescriptionVisible)
        return;

    const int lineHeight = 40;
    int nextEdgeListLength = 10;
    int baseX = int(window->getWidth() - Options::instance().infoPanelWidth);
    int baseY = 100;
    auto &state = states[animationState];
    edgeTextureFont->drawString("Next edges", ci::vec2(baseX + 30, baseY));
    auto it = state.notProcessed.begin();
    int line = 0;
    if (state.description.find("Check edge") == 0 || state.description.find("Joint two") == 0)
    {
        ci::gl::color(getColorScheme().highlightedEdgeColor1);
        edgeTextureFont->drawString(edgeToString(*state.inspectedEdge), ci::vec2(baseX, baseY + lineHeight));
        line++;
        nextEdgeListLength--;
    }

    for (int i = 0; i < nextEdgeListLength && it != state.notProcessed.end(); ++i, ++it, ++line)
    {
        auto &edge = **it;
        ci::gl::color(getColorScheme().edgeColor);
        if (state.inspectedEdge == *it)
        {
            ci::gl::color(getColorScheme().highlightedEdgeColor1);
        }

        edgeTextureFont->drawString(edgeToString(edge), ci::vec2(baseX, baseY + (line + 1) * lineHeight));
    }
    if (it != state.notProcessed.end())
    {
        edgeTextureFont->drawString("     ...", ci::vec2(baseX, baseY + (line + 1) * lineHeight));
    }
}



void KruskalDrawer::drawAlgorithmState()
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
    auto edgeParams = createDefaultEdgeParams();

    const ColorScheme &cs = getColorScheme();
    const float highlightedWidth = getDrawingSettings().highlightedEdgeWidth;
    for (const auto& e : state.mst)
    {
        edgeParams[e] = EdgeDrawParams(nodeGroupColors[state.uf.root(e->from)], highlightedWidth);        
    }

    for (const auto e : state.notProcessed)
    {
        edgeParams[e] = EdgeDrawParams(cs.edgeColor, getDrawingSettings().edgeWidth);
    }

    for (const auto e : state.notMst)
    {
        edgeParams[e] = EdgeDrawParams(cs.darkEdgeColor, getDrawingSettings().edgeWidth);
    }

    if (state.inspectedEdge != nullptr)
        edgeParams[state.inspectedEdge] = EdgeDrawParams(cs.highlightedEdgeColor1, highlightedWidth);

    drawEdges(edgeParams);

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(nodeGroupColors[state.uf.root(nodeIdx)]);
    }

    drawLabels(edgeParams);
    drawStepDescription(state.description);
    drawNextEdges();
}

void KruskalDrawer::createLegend()
{
    legend.clear();
    auto &cs = getColorScheme();
	legend.setBackgroundColor(cs.legendBackgroundColor);
    legend.add(LegendType::multiColorEdge, cs.edgeColor, "MST");
    legend.add(LegendType::highlightedEdge, cs.edgeColor, "Not processed");
    legend.add(LegendType::edge, cs.darkEdgeColor, "Not in MST");
    legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Inspected");
}

void KruskalDrawer::prepareAnimation()
{
    GraphAnimationDrawer::prepareAnimation();
    states = graph_algorithm_capture::mstKruskalCaptureStates(*g);
    animationLastState = int(states.size());
    if (nodeGroupColors.size() != g->getNodeCount())
    {
        nodeGroupColors = generateColors(g->getNodeCount());
        std::random_shuffle(begin(nodeGroupColors), end(nodeGroupColors));
    }
}

void KruskalDrawer::drawColorScale(const std::vector<ci::Color> &colorScale)
{
    if (g->getNodeCount() != 0 && !colorScale.empty())
    {
        const auto wHeight = float(window->getHeight());
        const auto size = float(window->getWidth()) / g->getNodeCount();
        for (int i = 0; i < g->getNodeCount(); ++i)
        {
            ci::gl::color(colorScale[i]);
            ci::gl::drawSolidRect(ci::Rectf(i*size, wHeight - size, (i + 1)*size, wHeight));
        }
    }
}


void KruskalDrawer::drawAlgorithmResult()
{
    startDrawing();
    drawEdges();
    auto edges = mstKruskal(*g);
    for (const auto &e : edges)
    {
        drawEdge(e->from, e->to, getColorScheme().highlightedEdgeColor2, getDrawingSettings().highlightedEdgeWidth);
    }
    drawNodes();
    drawLabels();
}
