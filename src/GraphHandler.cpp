#include "GraphHandler.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

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
    for (int i = 0; i < nodeHandlers.size(); ++i)
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
        ci::gl::color(ci::Color(1, 0.3, 0.7));
    }
    else
    {
        ci::gl::color(ci::Color(0.5, 0.5, 0.5));
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
