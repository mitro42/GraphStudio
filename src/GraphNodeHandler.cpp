#include "GraphNodeHandler.h"

#include "cinder/Rect.h"

const ci::Color GraphNodeHandler::NodeColors[3] = { ci::Color("red"), ci::Color::white(), ci::Color("orange") };

GraphNodeHandler::GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos, float size) : window(window), size(size), selection(none)
{
    float half = size / 2;
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
    ci::gl::color(NodeColors[int(selection)]);
    ci::gl::drawSolidCircle(rect.getCenter(), size);
}
