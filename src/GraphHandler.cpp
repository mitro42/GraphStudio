#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/CinderMath.h"
#include <fstream>
#include <string>
#include "Options.h"
#include "generators.h"


const int startNode = 16;

GraphHandler::GraphHandler() : g(true), oldNodeSize(0), forceType(none)
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
    /*
    in >> g;
    */
    int N, M;
    in >> N >> M;
    g.clear();
    g.setDirected(false);
    g.resize(N);
    g.setWeightedEdges(true);
    for (int i = 0; i < M; i++)
    {
        int a, b, w;
        in >> a >> b >> w;
        g.addEdge(a - 1, b - 1, w);
    }
    /*
    int N;
    g.clear();
    in >> N;
    g.resize(N + 1);
    g.setWeightedNodes(true);
    g.setWeightedEdges(false);
    for (int i = 0; i < N + 1; i++)
    {
        int deg;
        in >> deg;
        for (int d = 0; d < deg; d++)
        {
            int to;
            in >> to;
            g.addEdge(to - 1, i); // reverse the edges
        }
        g.getNode(i).setWeight(i+1);
    }

    for (int i = 0; i < N; i++)
    {
        float weight;
        in >> weight;
        g.getNode(i).setWeight(weight);
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
    auto gridSize = int(ceil(sqrt(g.getNodeCount())));
    auto nodePositions = generateGridPositions(g.getNodeCount(), gridSize, gridSize);
    repositionNodes(nodePositions);
}


void GraphHandler::repositionNodes(const std::vector<ci::Vec2f>& nodePositions)
{
    for (int i = 0; i < g.getNodeCount(); ++i)
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
    ci::gl::enableAlphaBlending();
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    if (!guard.try_lock())
        return;

    //fbo.bindFramebuffer();
    ci::gl::clear(Options::instance().backgroundColor);
    if (Options::instance().animationPlaying)
    {
        drawEdges();        
        drawNodes();
        drawAlgorithmStateDijkstra();
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
        drawLabels();
        //drawHighlightNodes();
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
            drawEdge(nodeIdx, node.getNeighbor(edgeIdx), false);
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


void GraphHandler::drawEdge(int from, int to, bool highlight)
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

}


void GraphHandler::drawHighlightEdges()
{
    auto edges = mstKruskal(g);
    for (const auto &e : edges)
    {
        drawEdge(e.from, e.to, true);
    }
    /*
    auto tree = edgeWeightDijkstra(g, startNode, -1);

    for (int i = 0; i < int(tree.size()); ++i)
    {
        auto from = tree[i].second;
        if (from == -1)
            continue;
        if (from == i)
            continue;
        drawEdge(from, i, tree[i].first - tree[from].first, true);
    }
    */
}


void GraphHandler::drawNodes()
{
    for (auto &nh : nodeHandlers)
    {
        nh->draw();
    }
}

void GraphHandler::drawLabels()
{
    if (oldNodeSize != Options::instance().nodeSize)
    {
        font = ci::Font("InputMono Black", Options::instance().nodeSize * 1.6f);
        textureFont = ci::gl::TextureFont::create(font);
        oldNodeSize = Options::instance().nodeSize;
    }

    // Nodes
    if (Options::instance().showNodeWeights)
    {
        for (int i = 0; i < g.getNodeCount(); ++i)
        {
            ci::gl::color(ci::ColorA(1, 1, 1, 1));
            auto label = std::to_string(i + 1);
            auto labelOffset = textureFont->measureString(label) / 2;
            labelOffset.y *= 0.65f;
            labelOffset.x *= -1;
            textureFont->drawString(label, nodeHandlers[i]->getPos() + labelOffset);
        }
    }

    // Edges
    if (Options::instance().showEdgeWeights && g.hasWeightedEdges())
    {
        auto edges = g.getEdges();

        for (const auto &edge : edges)
        {
            // writing edge weight
            ci::Vec2f fromVec = nodeHandlers[edge.from]->getPos();
            ci::Vec2f toVec = nodeHandlers[edge.to]->getPos();

            std::stringstream ss;
            ss << std::fixed << std::setprecision(Options::instance().weightPrecision) << edge.weight;
            std::string labelText = ss.str();
            ci::Vec2f textMid = (fromVec + toVec) / 2;
            ci::Vec2f edgeDir = fromVec - toVec;
            float deg = ci::toDegrees(std::atan(edgeDir.y / edgeDir.x));
            auto textRect = textureFont->measureString(labelText);
            float textWidth = textRect.x;
            ci::Vec2f textAlignmentOffset = edgeDir.normalized() * textWidth / 2.0f;
            if (fromVec.x < toVec.x)
            {
                textAlignmentOffset *= -1;
            }
            ci::gl::pushModelView();
            ci::gl::enableAlphaBlending();
            ci::gl::translate(textMid - textAlignmentOffset);
            if (edgeDir.x != 0)
            {
                ci::gl::rotate(deg);
            }
            else
            {
                ci::gl::rotate(90);
            }
            ci::Vec2f offset = -(fromVec + toVec).normalized() * 10; // place edge weight over the edge
            ci::gl::color(ci::ColorA(1, 1, 1, 1));
            textureFont->drawString(labelText, offset);
            ci::gl::popModelView();
        }
    }
}

void GraphHandler::drawHighlightNodes()
{
}


void GraphHandler::prepareAnimation()
{
    edgeWeightDijkstraStates = edgeWeightDijkstraCaptureStates(g, startNode, -1);
    animationState = 0;
    framesSpentInState = 0;
}

void GraphHandler::drawAlgorithmStateDijkstra()
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
        drawEdge(from, i, true);
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



void GraphHandler::generateSpecialGraph(GraphType type)
{
    std::cout << "GraphHandler::generateSpecialGraph(" << type << ")\n";
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);
    std::cout << "Start...\n";

    g.clear();
    std::vector<ci::Vec2f> nodePositions;
    switch (type)
    {
    case grid:
        generateGrid(GraphParamsGrid::instance(), g, nodePositions);
        break;
    case triangleMesh:
        generateTriangleMesh(GraphParamsTriangleMesh::instance(), g, nodePositions);
        break;
    case general:
    default:
        std::cout << "GraphHandler::generateSpecialGraph - SKIP\n";
    }

    recreateNodeHandlers(nodePositions);    
    std::cout << "End\n";


}