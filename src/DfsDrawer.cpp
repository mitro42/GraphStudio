#include "stdafx.h"
#include "DfsDrawer.h"
#include "Options.h"

const int DfsDrawer::infoPanelWidth;

void DfsDrawer::prepareNewState()
{

}


void DfsDrawer::drawAlgorithmState()
{
    setChanged();
    startDrawing();
    if (animationState >= int(states.size()))
        return;

	if (animationState == -1)
		return;

    auto &state = states[animationState];
    const float highlightedWidth = getDrawingSettings().highlightedEdgeWidth;
    const ColorScheme &cs = getColorScheme();

    auto edgeParams = createDefaultEdgeParams();

    for (const auto& e : state.currentPath)
    {
        edgeParams[e] = EdgeDrawParams(cs.highlightedEdgeColor2, getDrawingSettings().highlightedEdgeWidth);
    }

    drawEdges(edgeParams);

	std::vector<ci::Color> nodeColors(g->getNodeCount(), cs.nodeColor);
	for (int i = 0; i < state.finishedNodes.size(); ++i)
	{
		nodeColors[state.finishedNodes[i]] = nodeOrderColors[i];
	}

    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        nodeHandlers[nodeIdx]->draw(nodeColors[nodeIdx]);
    }
   
    drawLabels(edgeParams);
}

void DfsDrawer::createLegend()
{
    legend.clear();
 //   auto &cs = getColorScheme();
	//legend.setBackgroundColor(cs.legendBackgroundColor);
 //   legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor2, "MST");
 //   legend.add(LegendType::highlightedEdge, cs.darkEdgeColor, "Not in MST");
 //   legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor1, "Just found/checked");
 //   legend.add(LegendType::highlightedEdge, cs.highlightedEdgeColor3, "Undecided");
}


void DfsDrawer::prepareAnimation(int startNode)
{
    GraphAnimationDrawer::prepareAnimation(startNode);
    states = graph_algorithm_capture::depthFirstSearchCaptureStates(*g, startNode - 1);
	nodeOrderColors = generateColors(g->getNodeCount(), getColorScheme().highlightedEdgeColor1);
    animationLastState = int(states.size());
}

