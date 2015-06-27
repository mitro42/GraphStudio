#include "GraphNodeHandler.h"

#include "cinder/Rect.h"
#include "Options.h"

GraphNodeHandler::GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos) : window(window), selection(none)
{
    float half = Options::instance().nodeSize / 2;
    rect = ci::Rectf(pos + ci::Vec2f(-half, -half), pos + ci::Vec2f(half, half));

    cbMouseDown = window->getSignalMouseDown().connect(0, std::bind(&GraphNodeHandler::mouseDown, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(0, std::bind(&GraphNodeHandler::mouseUp, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(0, std::bind(&GraphNodeHandler::mouseDrag, this, std::placeholders::_1));
}



void GraphNodeHandler::mouseDrag(ci::app::MouseEvent &event)
{
    if (selection == move)
    {
        rect.offsetCenterTo(event.getPos());
    }
    event.setHandled(selection == move);
}


void GraphNodeHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (rect.contains(event.getPos()))
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



void GraphNodeHandler::draw()
{
    switch (selection)
    {
    case GraphNodeHandler::none:
        ci::gl::color(Options::instance().nodeColor);
        break;
    case GraphNodeHandler::move:
        ci::gl::color(Options::instance().movingNodeColor);
        break;
    case GraphNodeHandler::addEdge:
        ci::gl::color(Options::instance().addEdgeNodeColor);
        break;
    default:
        ci::gl::color(Options::instance().nodeColor);
        break;
    }
    ci::gl::drawSolidCircle(rect.getCenter(), Options::instance().nodeSize);
}
