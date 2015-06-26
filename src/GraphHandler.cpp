#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include <fstream>

GraphHandler::GraphHandler() : g(true), nodeRadius(10)
{

}


GraphHandler::~GraphHandler()
{
}

void GraphHandler::prepare(ci::app::WindowRef _window)
{
    window = _window;
    cbMouseDown = window->getSignalMouseDown().connect(std::bind(&GraphHandler::mouseDown, this, std::placeholders::_1));
}


void GraphHandler::update()
{
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


void GraphHandler::setup()
{
}

void GraphHandler::recreateNodeHandlers()
{
    nodeHandlers.clear();
    for (const auto &node : g)
    {
        auto pos = ci::Vec2f(ci::Rand::randFloat() * window->getWidth(), ci::Rand::randFloat() * window->getHeight());
        nodeHandlers.emplace_back(new GraphNodeHandler(window, pos, nodeRadius));
    }
}


void GraphHandler::loadGraph(std::string fileName)
{
    std::ifstream in(fileName);
    if (!in.good())
        throw("Cannot open file");
    in >> g;
    recreateNodeHandlers();
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


void GraphHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (event.isControlDown())
    {
        nodeHandlers.emplace_back(new GraphNodeHandler(window, event.getPos(), nodeRadius));
        g.addNode();
    }

    //for (int i = 0; i < nodeHandlers.size() - 1; ++i)
    //    edges.emplace_back(i, nodeHandlers.size() - 1);

    event.setHandled(true);
}


void GraphHandler::draw()
{
    //ci::gl::Fbo fbo(window->getWidth(), window->getHeight(), true);
    //fbo.bindFramebuffer();
    //ci::gl::clear(ci::ColorA(0, 0, 0));
    drawNodes();
    drawEdges();
    drawHighlightEdges();
    drawHighlightNodes();
    //fbo.unbindFramebuffer();
    //ci::gl::draw(fbo.getTexture());
}


void GraphHandler::drawEdges()
{
    for (int nodeIdx = 0; nodeIdx < g.getNodeCount(); ++nodeIdx)
    {
        for (const auto &e : g.getNode(nodeIdx))
        {
            drawEdge(nodeIdx, e, false);
        }
    }
}


void GraphHandler::drawEdge(int from, int to, bool highlight)
{
    if (highlight)
    {
        ci::gl::color(ci::Color(1.0f, 0.3f, 0.7f));
    }
    else
    {
        ci::gl::color(ci::Color(0.5f, 0.5f, 0.5f));
    }

    ci::Vec2f fromVec = nodeHandlers[from]->getPos();
    ci::Vec2f toVec = nodeHandlers[to]->getPos();
    if (g.isDirected())
    {
        ci::gl::drawVector(ci::Vec3f(fromVec, 0), ci::Vec3f(toVec, 0), 7, -5);
    }
    else
    {
        ci::gl::drawLine(fromVec, toVec);
    }
}


void GraphHandler::drawHighlightEdges()
{
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
