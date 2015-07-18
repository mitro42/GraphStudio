#include "GraphNodeHandler.h"

#include "cinder/Rect.h"
#include "Options.h"

GraphNodeHandler::GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos) : window(window), position(pos), selection(Selection::none)
{
    float half = Options::instance().nodeSize / 2;

    cbMouseDown = window->getSignalMouseDown().connect(0, std::bind(&GraphNodeHandler::mouseDown, this, std::placeholders::_1));
    cbMouseUp = window->getSignalMouseUp().connect(0, std::bind(&GraphNodeHandler::mouseUp, this, std::placeholders::_1));
    cbMouseDrag = window->getSignalMouseDrag().connect(0, std::bind(&GraphNodeHandler::mouseDrag, this, std::placeholders::_1));
}



void GraphNodeHandler::mouseDrag(ci::app::MouseEvent &event)
{
    if (selection == Selection::move)
    {
        position = event.getPos();
    }
    event.setHandled(selection == Selection::move);
}


void GraphNodeHandler::mouseDown(ci::app::MouseEvent &event)
{
    if ((position - event.getPos()).length() < Options::instance().nodeSize)
    {
        if (event.isAltDown())
        {
            selection = (selection == Selection::addEdge) ? Selection::none : Selection::addEdge;
        }
        else
        {
            selection = Selection::move;
        }
        event.setHandled(true);
    }
    else
    {
        if (selection == Selection::move)
            selection = Selection::none;
    }
}


void GraphNodeHandler::mouseUp(ci::app::MouseEvent &event)
{
    if (selection == Selection::move)
    {
        selection = Selection::none;
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
        color = Options::instance().currentColorScheme.highlightedNodeColor1;
    }
    else
    {
        switch (selection)
        {
        case Selection::none:
            color = Options::instance().currentColorScheme.nodeColor;
            break;
        case Selection::move:
            color = Options::instance().currentColorScheme.movingNodeColor;
            break;
        case Selection::addEdge:
            color = Options::instance().currentColorScheme.addEdgeNodeColor;
            break;
        default:
            color = Options::instance().currentColorScheme.nodeColor;
            break;
        }
    }
    draw(color);
}

