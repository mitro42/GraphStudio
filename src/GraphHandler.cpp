#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/CinderMath.h"
#include <fstream>

#include "Options.h"

GraphHandler::GraphHandler() : g(true)
{

}


GraphHandler::~GraphHandler()
{
}

void GraphHandler::prepare(ci::app::WindowRef _window)
{
    window = _window;
    cbMouseDown = window->getSignalMouseDown().connect(std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
    font = ci::Font("Arial", 22);
    textureFont = ci::gl::TextureFont::create(font);
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
            node.setEdgeWeight(j, (posStart - posEnd).length());
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
}


void GraphHandler::loadGraph(std::string fileName)
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    std::ifstream in(fileName);
    if (!in.good())
        throw("Cannot open file");
    in >> g;
    recreateNodeHandlers();

    /*
    auto N = int(sqrt(g.getNodeCount()));
    for (int i = 0; i < N * N; ++i)
    {
        if ((i + 1 <  N * N) && (i % N != N - 1))
        {
            g.addEdge(i, i + 1);
        }

        if (i + N <  N * N)
        {
            g.addEdge(i, i + N);
        }
    }
    */
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
    std::unique_lock<std::recursive_mutex> guard(updateMutex);
    auto N = int(ceil(sqrt(g.getNodeCount())));
    int row = 0;
    int col = 0;    
    int margin = 100;
    int width = window->getWidth() - (2 * margin);
    int height = window->getHeight() - (2 * margin);
    float xStep = float(width) / (N - 1);
    float yStep = float(height) / (N - 1);
    for (auto &nh: nodeHandlers)
    {
        nh->setPos(ci::Vec2f(margin + col * xStep, margin + row*yStep));
        col++;
        if (col == N)
        {
            col = 0;
            row++;
        }
    }
}



void GraphHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (event.isControlDown())
    {
        std::unique_lock<std::recursive_mutex> guard(updateMutex);
        nodeHandlers.emplace_back(new GraphNodeHandler(window, event.getPos()));
        g.addNode();
    }

    event.setHandled(true);
}


void GraphHandler::draw()
{
    std::unique_lock<std::recursive_mutex> guard(updateMutex, std::defer_lock);

    if (!guard.try_lock())
        return;

    //ci::gl::Fbo fbo(window->getWidth(), window->getHeight(), true);
    //fbo.bindFramebuffer();
    //ci::gl::clear(ci::ColorA(0, 0, 0));
    drawEdges();
    drawHighlightEdges();
    drawNodes();
    drawHighlightNodes();
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


void GraphHandler::drawEdge(int from, int to, double weight, bool highlight)
{
    if (from == to)
        return;

    // Setting the parameters
    if (highlight)
    {
        ci::gl::lineWidth(2);
        ci::gl::color(Options::instance().highlightedEdgeColor);
    }
    else
    {
        ci::gl::lineWidth(1);
        ci::gl::color(Options::instance().edgeColor);
    }

    // calculating the end points and drawing the lines/arrows
    ci::Vec2f fromVec = nodeHandlers[from]->getPos();
    ci::Vec2f toVec = nodeHandlers[to]->getPos();
    if (g.isDirected())
    {
        ci::Vec2f dir = toVec - fromVec;
        dir.normalize();
        ci::gl::drawVector(ci::Vec3f(fromVec, 0), 
            ci::Vec3f(toVec - (Options::instance().nodeSize + Options::instance().arrowSize) * dir, 0), 
            Options::instance().arrowSize, -5);
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


    /*
    for (const auto &e : edges)
    {
    if (e.first % 3 == 0)
    {

    }
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


void GraphHandler::drawHighlightNodes()
{
}
