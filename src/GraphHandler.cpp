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

GraphHandler::GraphHandler() : forceType(Force::none)
{
}


GraphHandler::~GraphHandler()
{
}


void GraphHandler::setup(ci::app::WindowRef _window)
{
    window = _window;
    cbMouseDown = window->getSignalMouseDown().connect(0, std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(0, std::bind(&GraphHandler::mouseDrag, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(0, std::bind(&GraphHandler::mouseUp, this, std::placeholders::_1));

    windowSize = window->getBounds();    
    g = std::make_shared<Graph>(true);
    graphDrawer = std::make_unique<NoAlgorithmDrawer>(g, nodeHandlers, window);
}


void GraphHandler::draw()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    if (!guard.try_lock())
        return;
    if (changed)
        graphDrawer->setChanged();

    graphDrawer->draw();
    changed = false;
}


void GraphHandler::animationPrepare()
{
    GraphAnimationDrawer* animationDrawer = dynamic_cast<GraphAnimationDrawer*>(graphDrawer.get());
    if (animationDrawer)
        animationDrawer->prepareAnimation();
}

void GraphHandler::animationPause()
{
    GraphAnimationDrawer* animationDrawer = dynamic_cast<GraphAnimationDrawer*>(graphDrawer.get());
    if (animationDrawer)
        animationDrawer->pause();
}

void GraphHandler::animationResume()
{
    GraphAnimationDrawer* animationDrawer = dynamic_cast<GraphAnimationDrawer*>(graphDrawer.get());
    if (animationDrawer)
        animationDrawer->resume();
}

bool GraphHandler::animationNext()
{
    GraphAnimationDrawer* animationDrawer = dynamic_cast<GraphAnimationDrawer*>(graphDrawer.get());
    if (animationDrawer)
        return animationDrawer->nextState();
    return false;
}

void GraphHandler::animationPrevious()
{
    GraphAnimationDrawer* animationDrawer = dynamic_cast<GraphAnimationDrawer*>(graphDrawer.get());
    if (animationDrawer)
        animationDrawer->previousState();
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
        for (auto &edgePtr: node)
        {
            auto neighbor = edgePtr->otherEnd(i);
            auto posEnd = nodeHandlers[neighbor]->getPos();
            auto newWeight = (glm::length(posStart - posEnd)) / 100 * Options::instance().edgeWeightScale;
            changed |= (edgePtr->weight != newWeight);
            edgePtr->weight = newWeight;
        }
    }
}

void GraphHandler::update()
{
    static ci::Perlin perlin;
    std::unique_lock<std::recursive_mutex> guard(updateMutex);  
    if(!Options::instance().animationPlaying)
    {
        addNewEdgeIfNodesSelected();
    }

    if (Options::instance().randomMovement)
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
    
    if (automaticEdgeWeightUpdate && !Options::instance().animationPlaying)
    {
        updateEdgeWeights();
    }


}


void GraphHandler::algorithmChanged()
{
    Options::instance().animationPlaying = false;
    switch (Algorithm(Options::instance().algorithm))
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
    graphDrawer->prepareAnimation();
}
void GraphHandler::recreateNodeHandlers()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (const auto &node : *g)
    {
        auto pos = ci::vec2(ci::Rand::randFloat() * window->getWidth(), ci::Rand::randFloat() * window->getHeight());
        nodeHandlers.emplace_back(new GraphNodeHandler(window, *this, pos));
    }
    changed = true;
}

void GraphHandler::recreateNodeHandlers(const std::vector<ci::vec2> &nodePositions)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers.emplace_back(new GraphNodeHandler(window, *this, nodePositions[i]));
    }
    changed = true;
}


void GraphHandler::loadGraph(std::string fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName);
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


void GraphHandler::saveGraph(std::string fileName)
{
    std::ofstream out(fileName);
    if (!out.good())
    {
        std::cout << "Cannot open graph file: [" << fileName << "]" << std::endl;
        return;
    }
    out << *g;
}


void GraphHandler::loadGraphPositions(std::string fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName);
    if (!in.good())
    {
        std::cout << "Cannot open positions file: [" << fileName << "]" << std::endl;
        return;
    }
        
    float x, y;
    int idx = 0;
    while (in >> x && idx < int(nodeHandlers.size()))
    {
        in >> y;
        nodeHandlers[idx++]->setPos(ci::vec2(x, y));
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


void GraphHandler::repositionNodes(const std::vector<ci::vec2>& nodePositions)
{
    for (int i = 0; i < g->getNodeCount(); ++i)
    {
        nodeHandlers[i]->setPos(nodePositions[i]);
    }
    changed = true;
}


void GraphHandler::pushNodes(ci::vec2 position, float force)
{
    for (auto &nh : nodeHandlers)
    {
        auto nodePosition = nh->getPos();
        auto forceVect = (position - nodePosition);
        nodePosition += glm::normalize(forceVect) * (force / glm::length2(forceVect) * 10.0f);
        nh->setPos(nodePosition);
    }
    changed = true;
}


void GraphHandler::pushNodes(ci::vec2 position)
{
    pushNodes(position, Options::instance().force);
}


void GraphHandler::pullNodes(ci::vec2 position)
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
    if (event.isControlDown() && !Options::instance().animationPlaying)
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        nodeHandlers.emplace_back(new GraphNodeHandler(window, *this, event.getPos()));
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
        nh->setPos(ci::vec2(pos.x * xScale, pos.y * yScale));
    }

    graphDrawer->resize(newWindowSize);

    windowSize = newWindowSize;
    changed = true;
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
    const float marginY = 0.1f;

    float targetHeight = float(window->getHeight());
    float targetWidth = float(window->getWidth());

    targetWidth *= (1 - 2 * marginX);
    targetHeight *= (1 - 2 * marginY);
    for (const auto& nh : nodeHandlers)
    {
        float newX = ((nh->getPos().x) - minX) / boundingRectWidth * window->getWidth() * (1 - 2 * marginX) + window->getWidth() * marginX;
        float newY = ((nh->getPos().y) - minY) / boundingRectHeight * window->getHeight() * (1 - 2 * marginY) + window->getHeight() * marginY;
        nh->setPos(ci::vec2(newX, newY));
    }
    setChanged();
}

void GraphHandler::generateSpecialGraph(GraphType type)
{
    std::cout << "GraphHandler::generateSpecialGraph(" << static_cast<int>(type) << ")\n";
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    std::cout << "Start...\n";

    std::vector<ci::vec2> nodePositions;
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
    changed = true;
    std::cout << "End\n";
}


void GraphHandler::setRandomEdgeWeights()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(int(Options::instance().minRandomEdgeWeight), int(Options::instance().maxRandomEdgeWeight));
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    for (auto node : *g)
    {
        for (auto edgePtr : *node)
        {
            edgePtr->weight = float(dis(gen));
        }
    }
    changed = true;
}