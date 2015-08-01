#include "KruskalDrawer.h"
#include "Options.h"

void KruskalDrawer::drawAlgorithmState()
{
    if (animationState >= int(states.size()))
        return;

    auto state = states[animationState];
    drawEdges();
    const ColorScheme &cs = Options::instance().currentColorScheme;
    const float highlightedWidth = Options::instance().highlighedEdgeWidth;
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
        nodeHandlers[nodeIdx]->draw(nodeGroupColors[state.uf.root(nodeIdx)]);
    }

    /*
    auto edges = (algo == mstKruskal(g);
    for (const auto &e : edges)
    {
        drawEdge(e.from, e.to, true);
    }
    */

    drawLabels();
}

void KruskalDrawer::prepareAnimation()
{
    states = graph_algorithm_capture::mstKruskalCaptureStates(*g);
    animationLastState = states.size();
    nodeGroupColors = generateColors(g->getNodeCount());
    std::random_shuffle(begin(nodeGroupColors), end(nodeGroupColors));
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