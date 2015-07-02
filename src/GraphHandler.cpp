#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/CinderMath.h"
#include <fstream>

#include "Options.h"

GraphHandler::GraphHandler() : g(true), forceType(none)
{

}


GraphHandler::~GraphHandler()
{
}

void GraphHandler::prepare(ci::app::WindowRef _window)
{
    window = _window;
    cbMouseDown = window->getSignalMouseDown().connect(std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(std::bind(&GraphHandler::mouseDrag, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(std::bind(&GraphHandler::mouseUp, this, std::placeholders::_1));
    font = ci::Font("Arial", 22);
    textureFont = ci::gl::TextureFont::create(font);
    ci::gl::Fbo::Format format;
    format.enableColorBuffer();
    format.setSamples( 4 );
    format.enableMipmapping();
    fbo = ci::gl::Fbo(window->getWidth(), window->getHeight(), format);
    windowSize = window->getBounds();    
}


void GraphHandler::addNewEdgeIfNodesSelected()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    int start = -1;
    int end = -1;
    for (size_t i = 0; i < nodeHandlers.size(); ++i)
    {
        if (nodeHandlers[i]->getSelection() == GraphNodeHandler::addEdge)
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
        g.addEdge(start, end);
        nodeHandlers[start]->clearSelection();
        nodeHandlers[end]->clearSelection();
        changed = true;
    }
}

void GraphHandler::updateEdgeWeights()
{    
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    for (size_t i = 0; i < g.getNodeCount(); ++i)
    {
        auto &node = g.getNode(i);
        auto posStart = nodeHandlers[i]->getPos();
        for (size_t j = 0; j < node.getNeighborCount(); ++j)
        {
            auto neighbor = node.getNeighbor(j);
            auto posEnd = nodeHandlers[neighbor]->getPos();
            auto newWeight = (posStart - posEnd).length();
            changed |= (node.getEdgeWeight(j) != newWeight);
            node.setEdgeWeight(j, newWeight);
        }
    }
}

void GraphHandler::update()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    addNewEdgeIfNodesSelected();
    if (automaticEdgeWeightUpdate)
    {
        updateEdgeWeights();
    }
}


void GraphHandler::setup()
{
}

void GraphHandler::recreateNodeHandlers()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    nodeHandlers.clear();
    for (const auto &node : g)
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
    for (int i = 0; i < g.getNodeCount(); ++i)
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
    in >> g;
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
    while (in >> x && idx < nodeHandlers.size())
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
    auto N = int(ceil(sqrt(g.getNodeCount())));
    reorderNodesGrid(N, N);
}

void GraphHandler::reorderNodesGrid(int columns, int rows)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);    
    int row = 0;
    int col = 0;    
    int margin = 100;
    int width = window->getWidth() - (2 * margin);
    int height = window->getHeight() - (2 * margin);
    float xStep = float(width) / (columns - 1);
    float yStep = float(height) / (rows - 1);
    for (auto &nh: nodeHandlers)
    {
        nh->setPos(ci::Vec2f(margin + col * xStep, margin + row*yStep));
        col++;
        if (col == columns)
        {
            col = 0;
            row++;
        }
    }
    changed = true;
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
    if (forceType != none)
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        if (forceType == push)
        {
            pushNodes(event.getPos());
        }
        else if (forceType == pull)
        {
            pullNodes(event.getPos());
        }
    }
}


void GraphHandler::mouseUp(ci::app::MouseEvent &event)
{
    forceType = none;
    //event.setHandled(true);
}

void GraphHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (event.isControlDown())
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        nodeHandlers.emplace_back(new GraphNodeHandler(window, event.getPos()));
        g.addNode();
        changed = true;
    }

    if (event.isAltDown())
    {
        if (event.isLeftDown())
        {
            forceType = push;
        }
        else if (event.isRightDown())
        {
            forceType = pull;
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


void GraphHandler::draw()
{
    //if (!changed)
    //    return;
    
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    if (!guard.try_lock())
        return;

    //fbo.bindFramebuffer();
    ci::gl::clear(Options::instance().backgroundColor);
    if (Options::instance().animationPlaying)
    {
        drawEdges();        
        drawNodes();
        drawAlgorithmState();
        if (animationState < edgeWeightDijkstraStates.size()-1)
        {
            framesSpentInState++;
            if (framesSpentInState % Options::instance().speed == 0)
                animationState++;
        }
    }
    else
    {
        drawEdges();
        drawHighlightEdges();
        drawNodes();
        drawHighlightNodes();
    }

    
    changed = false;
    
    //fbo.unbindFramebuffer();
    //ci::gl::draw(fbo.getTexture());
}


void GraphHandler::drawEdges()
{
    for (int nodeIdx = 0; nodeIdx < g.getNodeCount(); ++nodeIdx)
    {
        const auto &node = g.getNode(nodeIdx);
        for (int edgeIdx = 0; edgeIdx < node.getNeighborCount(); ++edgeIdx)
        {
            drawEdge(nodeIdx, node.getNeighbor(edgeIdx), node.getEdgeWeight(edgeIdx), false);
        }
    }
}

void GraphHandler::drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle)
{
    ci::gl::drawLine(from, to);
    ci::Vec2f dir = (to - from).normalized();
    dir.rotate(ci::toRadians(headAngle));
    ci::gl::drawLine(to, to - dir*headLength);
    dir.rotate(ci::toRadians(-2*headAngle));
    ci::gl::drawLine(to, to - dir*headLength);
}


void GraphHandler::drawEdge(int from, int to, double weight, bool highlight)
{
    if (from == to)
        return;

    // Setting the parameters
    if (highlight)
    {
        ci::gl::lineWidth(Options::instance().highlighedEdgeWidth);
        ci::gl::color(Options::instance().highlightedEdgeColor);
    }
    else
    {
        ci::gl::lineWidth(Options::instance().edgeWidth);
        ci::gl::color(Options::instance().edgeColor);
    }

    // calculating the end points and drawing the lines/arrows
    ci::Vec2f fromVec = nodeHandlers[from]->getPos();
    ci::Vec2f toVec = nodeHandlers[to]->getPos();
    if (g.isDirected())
    {
        ci::Vec2f dir = toVec - fromVec;
        dir.normalize();
        drawArrow(fromVec, toVec - Options::instance().nodeSize * dir, 
            Options::instance().arrowLength, Options::instance().arrowAngle);
    }
    else
    {
        ci::gl::drawLine(fromVec, toVec);
    }


    // writing edge weight
    if (g.hasWeightedEdges())
    {
        std::stringstream ss;
        ss << std::fixed << std::setw(8) << std::setprecision(2) << weight;
        ci::Vec2f textPos = (fromVec + toVec) / 2;
        ci::Vec2f offset = (fromVec + toVec).normalized();
        offset.rotate(float(M_PI));

        ci::gl::pushModelView();
        //ci::gl::translate(ci::Vec3f(window->getWidth() / 2, window->getHeight() / 2, 0));
        //ci::gl::translate(ci::Vec3f(500, 100, 0));
        ci::gl::translate(textPos);
        ci::gl::rotate(0);
        textureFont->drawString(ss.str(), offset * 20);

        ci::gl::popModelView();
    }
}


void GraphHandler::drawHighlightEdges()
{

    auto tree = edgeWeightDijkstra(g, 0, -1);

    for (int i = 0; i < int(tree.size()); ++i)
    {
        auto from = tree[i].second;
        if (from == -1)
            continue;
        if (from == i)
            continue;
        drawEdge(from, i, tree[i].first - tree[from].first, true);
    }
}


void GraphHandler::drawNodes()
{
    for (auto &nh : nodeHandlers)
    {
        nh->draw();
    }
}


void GraphHandler::drawHighlightNodes()
{
}


void GraphHandler::prepareAnimation()
{
    edgeWeightDijkstraStates = edgeWeightDijkstraCaptureStates(g, 0, -1);
    animationState = 0;
    framesSpentInState = 0;
}

void GraphHandler::drawAlgorithmState()
{
    auto state = edgeWeightDijkstraStates[animationState];
    auto tree = state.first;
    auto q = state.second;

    for (int i = 0; i < int(tree.size()); ++i)
    {
        auto from = tree[i].second;
        if (from == -1)
            continue;
        if (from == i)
            continue;
        drawEdge(from, i, tree[i].first - tree[from].first, true);
    }

    std::vector<bool> nodeHighlight(g.getNodeCount(), false);
    for (auto &p : q)
    {
        nodeHighlight[p.second] = true;
    }

    for (int i = 0; i < g.getNodeCount(); ++i)
    {
        nodeHandlers[i]->draw(nodeHighlight[i]);
    }
}

void GraphHandler::generateGrid()
{
    std::cout << "GraphHandler::generateGrid\n";
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    std::cout << "Start...\n";
    g.clear();
    const auto &params = GraphParamsGrid::instance();
    g.setDirected(params.directed);
    g.setWeightedEdges(false);
    g.setWeightedNodes(false);
    for (int i = 0; i < params.rows; i++)
    {
        for (int j = 0; j < params.columns; j++)
        {
            g.addNode();
        }
    }

    if (params.horizontal)
    {
        for (int i = 0; i < params.rows; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1);
            }
        }
    }

    if (params.vertical)
    {
        for (int i = 0; i < params.rows - 1; i++)
        {
            for (int j = 0; j < params.columns; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + params.columns);
            }
        }
    }

    if (params.upDiagonal)
    {
        for (int i = 1; i < params.rows; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1 - params.columns);
            }
        }
    }

    if (params.downDiagonal)
    {
        for (int i = 0; i < params.rows - 1; i++)
        {
            for (int j = 0; j < params.columns - 1; j++)
            {
                int start = i*params.columns + j;
                g.addEdge(start, start + 1 + params.columns);
            }
        }
    }
    recreateNodeHandlers();
    reorderNodesGrid(params.columns, params.rows);
    std::cout << "End\n";
}


void GraphHandler::generateTriangleMesh()
{
    std::vector<std::pair<int, int>> outerEdges;
    std::vector<ci::Vec2f> nodePositions;
    ci::randSeed(42);
    g.setDirected(false);
    g.setWeightedEdges(false);
    g.setWeightedNodes(false);
    g.clear(); 
    g.addNode();
    g.addNode();
    g.addEdge(0, 1);
    outerEdges.emplace_back(0, 1);
    ci::Vec2f center(window->getWidth() / 2, window->getHeight() / 2);
    ci::Vec2f centerOffset = ci::randVec2f() * 25;
    nodePositions.push_back(center + centerOffset);
    nodePositions.push_back(center - centerOffset);
    while (nodePositions.size() < GraphParamsTriangleMesh::instance().triangles)
    {
        int edgeIdx = ci::randInt(outerEdges.size());
        auto edge = outerEdges[edgeIdx];
        int newIdx = g.addNode();
        g.addEdge(newIdx, edge.first);
        g.addEdge(newIdx, edge.second);
        outerEdges.emplace_back(newIdx, edge.first);
        outerEdges.emplace_back(newIdx, edge.second);
        size_t size = outerEdges.size();
        std::swap(outerEdges[edgeIdx], outerEdges[size-1]);
        outerEdges.resize(size - 1);
        ci::Vec2f offset = (nodePositions[edge.first] - nodePositions[edge.second]).normalized();
        offset.rotate(M_PI / 2);
        ci::Vec2f newPos = (nodePositions[edge.first] + nodePositions[edge.second]) / 2;
        newPos += offset * sqrt(3) * 50;
        nodePositions.push_back(newPos);
    }
    recreateNodeHandlers(nodePositions);
}