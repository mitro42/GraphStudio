#include "stdafx.h"
#include "GraphHandler.h"

#include "NoAlgorithmDrawer.h"
#include "DijkstraDrawer.h"
#include "KruskalDrawer.h"
#include "PrimDrawer.h"
#include "Options.h"

#include "generators.h"
#include <random>

#include <cinder/Perlin.h>
#include <cinder/Rand.h>
float GraphNodeHandler::size;


void GraphHandler::setup(ci::app::WindowRef _window)
{
    window = _window;
	connectMouseEvents();
    windowSize = window->getBounds();    
    g = std::make_shared<Graph>(true);
    graphDrawer = std::make_unique<NoAlgorithmDrawer>(g, nodeHandlers, windowSize);
}


void GraphHandler::connectMouseEvents()
{
	cbMouseDown = window->getSignalMouseDown().connect(0, std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
	cbMouseUp = window->getSignalMouseUp().connect(0, std::bind(&GraphHandler::mouseUp, this, std::placeholders::_1));
	for (auto &nh : nodeHandlers)
	{
		nh->connectMouseEvents();
	}
}


void GraphHandler::disconnectMouseEvents()
{
	cbMouseDown.disconnect();
	cbMouseUp.disconnect();
	for (auto &nh : nodeHandlers)
	{
		nh->disconnectMouseEvents();
	}
}

bool GraphHandler::nodeHandlersChanged()
{
	bool ret = false;
	for (auto &nh : nodeHandlers)
	{
		ret |= nh->isChanged();
	}
	return ret;
}

void GraphHandler::clearChanged()
{
	changed = false;
	for (auto &nh : nodeHandlers)
	{
		nh->clearChanged();
	}
}

void GraphHandler::draw()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    if (!guard.try_lock())
        return;
    if (nodeHandlersChanged() || changed)
        graphDrawer->setChanged();

    graphDrawer->draw();
	clearChanged();
}


void GraphHandler::animationPrepare(int startNode)
{
	graphDrawer->prepareAnimation(startNode);
}

void GraphHandler::animationPause()
{
	graphDrawer->pause();
}

void GraphHandler::animationResume()
{
	graphDrawer->resume();
}

bool GraphHandler::animationNext()
{
    return graphDrawer->nextState();
}

void GraphHandler::animationPrevious()
{
	graphDrawer->previousState();
}

void GraphHandler::animationGoToFirst()
{
	graphDrawer->animationGoToFirst();
}

void GraphHandler::animationGoToLast()
{
	graphDrawer->animationGoToLast();
}


void GraphHandler::addNewEdgeIfNodesSelected()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    int start = -1;
    int end = -1;
    for (int i = 0; i < int(nodeHandlers.size()); ++i)
    {
        if (nodeHandlers[i]->getSelection() == GraphNodeHandler::Selection::addEdge)
        {
            if (start == -1)
            {
                start = i;
            }
            else
            {
                end = i;
                break;
            }
        }
    }
    if (start != -1 && end != -1)
    {
        if (nodeHandlers[start]->getSelectedInFrame() > nodeHandlers[end]->getSelectedInFrame())
        {
            std::swap(start, end);
        }
        //ci::app::console() << "New edge: " << start << " - " << end << std::endl;
        g->addEdge(start, end);
        nodeHandlers[start]->clearSelection();
        nodeHandlers[end]->clearSelection();
        setChanged();
    }
}

void GraphHandler::setEdgeWeightsFromLengths(double scale)
{    
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        auto &node = g->getNode(i);
        auto posStart = nodeHandlers[i]->getPos();
        for (auto &edge: node)
        {
            auto neighbor = edge.otherEnd(i);
            auto posEnd = nodeHandlers[neighbor]->getPos();
            auto newWeight = (glm::length(posStart - posEnd)) * scale;
            changed |= (edge.weight != newWeight);
            edge.weight = newWeight;
        }
    }
}

void GraphHandler::update()
{
    static ci::Perlin perlin;
    std::unique_lock<std::recursive_mutex> guard(updateMutex);  
    addNewEdgeIfNodesSelected();

    if (randomMovement)
    {
        for (auto& nh : nodeHandlers)
        {
            ci::vec2 v;
            v.x = nh->getOriginalPos().x + float(ci::app::getElapsedSeconds()) * 0.1f;
            v.y = nh->getOriginalPos().y + float(ci::app::getElapsedSeconds()) * 0.01f;
            ci::vec2 randVec = perlin.dfBm(v);
            //ci::vec2 newSpeed(sin(angle), cos(angle));
            nh->setSpeed(randVec.x * 3);
            nh->setDirection(randVec.y);
            nh->update();
        }
        setChanged();
    }
}


void GraphHandler::algorithmChanged(Algorithm newAlgorithm, int startNode)
{
    switch (newAlgorithm)
    {
    default:
    case Algorithm::none:
        graphDrawer = std::make_unique<NoAlgorithmDrawer>(g, nodeHandlers, windowSize);
        break;
    case Algorithm::prim:
        graphDrawer = std::make_unique<PrimDrawer>(g, nodeHandlers, windowSize);
        break;
    case Algorithm::kruskal:
        graphDrawer = std::make_unique<KruskalDrawer>(g, nodeHandlers, windowSize);
        break;
    case Algorithm::dijkstra:
        graphDrawer = std::make_unique<DijkstraDrawer>(g, nodeHandlers, windowSize);
        break;
    }
    
	graphDrawer->prepareAnimation(startNode);
    graphDrawer->animationGoToLast();
}

void GraphHandler::algorithmStartNodeChanged(int startNode)
{
    graphDrawer->prepareAnimation(startNode);
}

void GraphHandler::addNodeHandler(ci::vec2 pos)
{	
	nodePositions.push_back(std::make_unique<ci::vec2>(pos));
	nodeHandlers.push_back(std::make_unique<GraphNodeHandler>(window, nodePositions.back().get()));
}

void GraphHandler::recreateNodeHandlers()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (const auto &node : *g)
    {
		auto newPos = ci::vec2(ci::Rand::randFloat() * window->getWidth(), ci::Rand::randFloat() * window->getHeight());
		addNodeHandler(newPos);
    }
    setChanged();
}




void GraphHandler::recreateNodeHandlers(const std::vector<ci::vec2> &newNodePositions)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
		addNodeHandler(newNodePositions[i]);
    }
    setChanged();
}


void GraphHandler::loadGraph(const ci::fs::path &fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName.string());
    if (!in.good())
        throw("Cannot open file");
    in >> *g;
    /*
    int N, M;
    in >> N >> M;
    g->clear();
    g->setDirected(true);
    g->resize(N);
    g->setWeightedEdges(true);
    for (int i = 0; i < M; i++)
    {
        int a, b, w;
        in >> a >> b >> w;
        g->addEdge(a - 1, b - 1, w);
    }
    */
    /*
    int N;
    g->clear();
    in >> N;
    g->resize(N + 1);
    g->setWeightedNodes(true);
    g->setWeightedEdges(false);
    for (int i = 0; i < N + 1; i++)
    {
        int deg;
        in >> deg;
        for (int d = 0; d < deg; d++)
        {
            int to;
            in >> to;
            g->addEdge(to - 1, i); // reverse the edges
        }
        g->getNode(i).setWeight(i+1);
    }

    for (int i = 0; i < N; i++)
    {
        float weight;
        in >> weight;
        g->getNode(i).setWeight(weight);
    }   
    */

    recreateNodeHandlers();
    setChanged();
}


void GraphHandler::saveGraph(const ci::fs::path &fileName)
{
    std::ofstream out(fileName.string());
    if (!out.good())
    {
        ci::app::console() << "Cannot open graph file: [" << fileName << "]" << std::endl;
        return;
    }
    out << *g;
}


void GraphHandler::loadGraphPositions(const ci::fs::path &fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName.string());
    if (!in.good())
    {
        ci::app::console() << "Cannot open positions file: [" << fileName << "]" << std::endl;
        return;
    }
        
    float x, y;
    int idx = 0;
    while (in >> x && idx < int(nodeHandlers.size()))
    {
        in >> y;
		*nodePositions[idx++] = ci::vec2{ x, y };
    }
    setChanged();
}


void GraphHandler::saveGraphPositions(const ci::fs::path &fileName)
{
    std::ofstream out(fileName.string());
    if (!out.good())
        throw("Cannot open file");
    for (const auto& node : nodeHandlers)
    {
        const auto &pos = node->getPos();
        out << pos.x << " " << pos.y << "\n";
    }
    out << std::flush;
}

void GraphHandler::reorderNodesSquare()
{
    auto gridSize = int(ceil(sqrt(g->getNodeCount())));
    auto nodePositions = generateGridPositions(g->getNodeCount(), gridSize, gridSize);
    repositionNodes(nodePositions);
}


void GraphHandler::repositionNodes(const std::vector<ci::vec2>& newNodePositions)
{
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
		*nodePositions[i] = newNodePositions[i];
    }
    setChanged();
}


void GraphHandler::mouseUp(ci::app::MouseEvent &event)
{
    //event.setHandled(true);
}

void GraphHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (event.isControlDown())
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
		addNodeHandler(event.getPos());
        g->addNode();
        setChanged();
    }

    event.setHandled(true);
}

void GraphHandler::resize(ci::Area newWindowSize)
{
    float xScale = float(newWindowSize.getWidth()) / windowSize.getWidth();
    float yScale = float(newWindowSize.getHeight()) / windowSize.getHeight();
    
    for (auto &nodePos : nodePositions)
    {        
        *nodePos = (ci::vec2(nodePos->x * xScale, nodePos->y * yScale));
    }

    graphDrawer->resize(newWindowSize);

    windowSize = newWindowSize;
    setChanged();
}



void GraphHandler::fitToWindow()
{
    float minX, maxX, minY, maxY;
    minX = minY = std::numeric_limits<float>::max();
    maxX = maxX = std::numeric_limits<float>::min();
    
    for (const auto& nh : nodeHandlers)
    {
        auto pos = nh->getPos();
        minX = std::min(minX, pos.x);
        minY = std::min(minY, pos.y);
        maxX = std::max(maxX, pos.x);
        maxY = std::max(maxY, pos.y);
    }

    const float boundingRectWidth = maxX - minX;
    const float boundingRectHeight = maxY - minY;
    const float midX = (maxX + minX) / 2;
    const float midY = (maxY + minY) / 2;

    const float marginX = 0.05f;
    const float marginY = 0.1f;

    float targetHeight = float(window->getHeight());
    float targetWidth = float(window->getWidth());

    targetWidth *= (1 - 2 * marginX);
    targetHeight *= (1 - 2 * marginY);
    for (auto &nodePos : nodePositions)
    {
        float newX = (nodePos->x - minX) / boundingRectWidth * targetWidth + window->getWidth() * marginX;
        float newY = (nodePos->y - minY) / boundingRectHeight * targetHeight + window->getHeight() * marginY;
        *nodePos = ci::vec2(newX, newY);
    }
    setChanged();
}


void GraphHandler::generateSpecialGraph(const GraphGenerator &generator)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);

    std::vector<ci::vec2> nodePositions;
    generator.run(*g, nodePositions);

    recreateNodeHandlers(nodePositions);
    setChanged();
}


void GraphHandler::setRandomEdgeWeights(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
	if (max < min) std::swap(max, min);
    std::uniform_int_distribution<int> dis(min, max);
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    for (auto node : *g)
    {
        for (auto &edge : node)
        {
            edge.weight = float(dis(gen));
        }
    }
    setChanged();
}