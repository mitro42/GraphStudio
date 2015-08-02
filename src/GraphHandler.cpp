#include "stdafx.h"
#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/CinderMath.h"
#include <fstream>
#include <string>

#include "NoAlgorithmDrawer.h"
#include "DijkstraDrawer.h"
#include "KruskalDrawer.h"
#include "PrimDrawer.h"
#include "Options.h"

#include "generators.h"


GraphHandler::GraphHandler() : forceType(Force::none)
{
}


GraphHandler::~GraphHandler()
{
}


void GraphHandler::setup(ci::app::WindowRef _window)
{
    window = _window;
    cbMouseDown = window->getSignalMouseDown().connect(std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(std::bind(&GraphHandler::mouseDrag, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(std::bind(&GraphHandler::mouseUp, this, std::placeholders::_1));

    windowSize = window->getBounds();    
    g = std::make_shared<Graph>(true);
    graphDrawer = std::make_unique<NoAlgorithmDrawer>(g, nodeHandlers, window);
}


void GraphHandler::draw()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    if (!guard.try_lock())
        return;

    graphDrawer->draw();

}


void GraphHandler::prepareAnimation()
{
    algorithmAnimationMode = true;
    algorithmAnimationPlaying = true;
    graphDrawer->prepareAnimation();
}




void GraphHandler::addNewEdgeIfNodesSelected()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    int start = -1;
    int end = -1;
    for (size_t i = 0; i < nodeHandlers.size(); ++i)
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
        //std::cout << "New edge: " << start << " - " << end << std::endl;
        g->addEdge(start, end);
        nodeHandlers[start]->clearSelection();
        nodeHandlers[end]->clearSelection();
        changed = true;
    }
}

void GraphHandler::updateEdgeWeights()
{    
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        auto &node = g->getNode(i);
        auto posStart = nodeHandlers[i]->getPos();
        for (int j = 0; j < node.getNeighborCount(); ++j)
        {
            auto neighbor = node.getNeighbor(j);
            auto posEnd = nodeHandlers[neighbor]->getPos();
            auto newWeight = ((posStart - posEnd).length()) / 100 * Options::instance().edgeWeightScale;
            changed |= (node.getEdgeWeight(j) != newWeight);
            node.setEdgeWeight(j, newWeight);
        }
    }
}

void GraphHandler::update()
{
    static ci::Perlin perlin;
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    addNewEdgeIfNodesSelected();
    if (Options::instance().randomMovement)
    {
        for (auto& nh : nodeHandlers)
        {
            ci::Vec2f v;
            v.x = nh->getOriginalPos().x + float(ci::app::getElapsedSeconds()) * 0.1f;
            v.y = nh->getOriginalPos().y + float(ci::app::getElapsedSeconds()) * 0.01f;
            ci::Vec2f randVec = perlin.dfBm(v);
            //ci::Vec2f newSpeed(sin(angle), cos(angle));
            nh->setSpeed(randVec.x * 3);
            nh->setDirection(randVec.y);
            nh->update();
        }
    }
    if (automaticEdgeWeightUpdate)
    {
        updateEdgeWeights();
    }
    if (currentAlgorithm != Options::instance().algorithm)
    {
        currentAlgorithm = Options::instance().algorithm;
        switch (Algorithm(currentAlgorithm))
        {
        default:
        case Algorithm::none:
            graphDrawer = std::make_unique<NoAlgorithmDrawer>(g, nodeHandlers, window);
            break;
        case Algorithm::prim:
            graphDrawer = std::make_unique<PrimDrawer>(g, nodeHandlers, window);
            break;
        case Algorithm::kruskal:
            graphDrawer = std::make_unique<KruskalDrawer>(g, nodeHandlers, window);
            break;
        case Algorithm::dijkstra:
            graphDrawer = std::make_unique<DijkstraDrawer>(g, nodeHandlers, window);
            break;
        }
    }

}

void GraphHandler::recreateNodeHandlers()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (const auto &node : *g)
    {
        auto pos = ci::Vec2f(ci::Rand::randFloat() * window->getWidth(), ci::Rand::randFloat() * window->getHeight());
        nodeHandlers.emplace_back(new GraphNodeHandler(window, pos));
    }
    changed = true;
}

void GraphHandler::recreateNodeHandlers(const std::vector<ci::Vec2f> &nodePositions)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers.emplace_back(new GraphNodeHandler(window, nodePositions[i]));
    }
    changed = true;
}


void GraphHandler::loadGraph(std::string fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName);
    if (!in.good())
        throw("Cannot open file");
    /*
    in >> g;
    */
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
    changed = true;
}


void GraphHandler::saveGraph(std::string fileName)
{
    std::ofstream out(fileName);
    if (!out.good())
        throw("Cannot open file");
    out << g;
}


void GraphHandler::loadGraphPositions(std::string fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName);
    if (!in.good())
        throw("Cannot open file");
    float x, y;
    int idx = 0;
    while (in >> x && idx < int(nodeHandlers.size()))
    {
        in >> y;
        nodeHandlers[idx++]->setPos(ci::Vec2f(x, y));
    }
    changed = true;
}


void GraphHandler::saveGraphPositions(std::string fileName)
{
    std::ofstream out(fileName);
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


void GraphHandler::repositionNodes(const std::vector<ci::Vec2f>& nodePositions)
{
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers[i]->setPos(nodePositions[i]);
    }
}


void GraphHandler::pushNodes(ci::Vec2f position, float force)
{
    for (auto &nh : nodeHandlers)
    {
        auto nodePosition = nh->getPos();
        auto forceVect = (position - nodePosition);
        nodePosition += forceVect.normalized() * (force / forceVect.lengthSquared()) * 10;
        nh->setPos(nodePosition);
    }
}


void GraphHandler::pushNodes(ci::Vec2f position)
{
    pushNodes(position, Options::instance().force);
}


void GraphHandler::pullNodes(ci::Vec2f position)
{
    pushNodes(position, -Options::instance().force);
}

void GraphHandler::mouseDrag(ci::app::MouseEvent &event)
{
    if (forceType != Force::none)
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        if (forceType == Force::push)
        {
            pushNodes(event.getPos());
        }
        else if (forceType == Force::pull)
        {
            pullNodes(event.getPos());
        }
    }
}

void GraphHandler::mouseUp(ci::app::MouseEvent &event)
{
    forceType = Force::none;
    //event.setHandled(true);
}

void GraphHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (event.isControlDown())
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        nodeHandlers.emplace_back(new GraphNodeHandler(window, event.getPos()));
        g->addNode();
        changed = true;
    }

    if (event.isAltDown())
    {
        if (event.isLeftDown())
        {
            forceType = Force::push;
        }
        else if (event.isRightDown())
        {
            forceType = Force::pull;
        }
    }

    event.setHandled(true);
}

void GraphHandler::resize(ci::Area newWindowSize)
{
    float xScale = float(newWindowSize.getWidth()) / windowSize.getWidth();
    float yScale = float(newWindowSize.getHeight()) / windowSize.getHeight();
    
    for (auto &nh : nodeHandlers)
    {
        auto pos = nh->getPos();
        nh->setPos(ci::Vec2f(pos.x * xScale, pos.y * yScale));
    }

    windowSize = newWindowSize;
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

    const float marginX = 0.1f;
    const float marginY = 0.05f;

    float targetHeight = float(window->getHeight());
    float targetWidth = float(window->getWidth());

    targetWidth *= (1 - 2 * marginX);
    targetHeight *= (1 - 2 * marginY);
    for (const auto& nh : nodeHandlers)
    {
        float newX = ((nh->getPos().x) - minX) / boundingRectWidth * window->getWidth() * (1 - 2 * marginX) + window->getWidth() * marginX;
        float newY = ((nh->getPos().y) - minY) / boundingRectHeight * window->getHeight() * (1 - 2 * marginY) + window->getHeight() * marginY;
        nh->setPos(ci::Vec2f(newX, newY));
    }
}

void GraphHandler::generateSpecialGraph(GraphType type)
{
    std::cout << "GraphHandler::generateSpecialGraph(" << static_cast<int>(type) << ")\n";
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    std::cout << "Start...\n";

    g->clear();
    std::vector<ci::Vec2f> nodePositions;
    switch (type)
    {
    case GraphType::grid:
        generateGrid(GraphParamsGrid::instance(), *g, nodePositions);
        break;
    case GraphType::triangleMesh:
        generateTriangleMesh(GraphParamsTriangleMesh::instance(), *g, nodePositions);
        break;
    case GraphType::general:
    default:
        std::cout << "GraphHandler::generateSpecialGraph - SKIP\n";
    }
    recreateNodeHandlers(nodePositions);
    std::cout << "End\n";
}

