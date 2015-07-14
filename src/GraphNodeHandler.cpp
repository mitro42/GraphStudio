#include "GraphNodeHandler.h"

#include "cinder/Rect.h"
#include "Options.h"

GraphNodeHandler::GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos) : window(window), position(pos), selection(none)
{
    float half = Options::instance().nodeSize / 2;

    cbMouseDown = window->getSignalMouseDown().connect(0, std::bind(&GraphNodeHandler::mouseDown, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(0, std::bind(&GraphNodeHandler::mouseUp, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(0, std::bind(&GraphNodeHandler::mouseDrag, this, std::placeholders::_1));
}



void GraphNodeHandler::mouseDrag(ci::app::MouseEvent &event)
{
    if (selection == move)
    {
        position = event.getPos();
    }
    event.setHandled(selection == move);
}


void GraphNodeHandler::mouseDown(ci::app::MouseEvent &event)
{
    if ((position - event.getPos()).length() < Options::instance().nodeSize)
    {
        if (event.isAltDown())
        {
            selection = (selection == addEdge) ? none : addEdge;
        }
        else
        {
            selection = move;
        }
        event.setHandled(true);
    }
    else
    {
        if (selection == move)
            selection = none;
    }
}


void GraphNodeHandler::mouseUp(ci::app::MouseEvent &event)
{
    if (selection == move)
    {
        selection = none;
        event.setHandled(true);
        return;
    }
    event.setHandled(false);
}


void GraphNodeHandler::draw(const ci::Color &color)
{
    ci::gl::color(color);
    ci::gl::drawSolidCircle(position, Options::instance().nodeSize);
}


void GraphNodeHandler::draw(bool highlighted)
{
    ci::Color color;
    if (highlighted)
    {
        color = Options::instance().highlightedNodeColor;
    }
    else
    {
        switch (selection)
        {
        case GraphNodeHandler::none:
            color = Options::instance().nodeColor;
            break;
        case GraphNodeHandler::move:
            color = Options::instance().movingNodeColor;
            break;
        case GraphNodeHandler::addEdge:
            color = Options::instance().addEdgeNodeColor;
            break;
        default:
            color = Options::instance().nodeColor;
            break;
        }
    }
    draw(color);
}

