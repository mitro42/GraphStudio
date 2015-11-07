#include "stdafx.h"
#include "GraphNodeHandler.h"

const float GraphNodeHandler::margin = 10.0f;

GraphNodeHandler::GraphNodeHandler(ci::app::WindowRef window, ci::vec2 pos) : 
    window(window), 
    position(pos), 
    originalPosition(pos), 
    direction(0.0f),
    speed(0.0f),
    selection(Selection::none)
{
	connectMouseEvents();
}

void GraphNodeHandler::connectMouseEvents()
{
	cbMouseDown = window->getSignalMouseDown().connect(2, std::bind(&GraphNodeHandler::mouseDown, this, std::placeholders::_1));
	cbMouseUp = window->getSignalMouseUp().connect(2, std::bind(&GraphNodeHandler::mouseUp, this, std::placeholders::_1));
	cbMouseDrag = window->getSignalMouseDrag().connect(2, std::bind(&GraphNodeHandler::mouseDrag, this, std::placeholders::_1));
}

void GraphNodeHandler::disconnectMouseEvents()
{
	cbMouseDown.disconnect();
	cbMouseUp.disconnect();
	cbMouseDrag.disconnect();
}



void GraphNodeHandler::mouseDrag(ci::app::MouseEvent &event)
{
    if (selection == Selection::move)
    {
        position = event.getPos();
		changed = true;
    }
    event.setHandled(selection == Selection::move);
}


void GraphNodeHandler::mouseDown(ci::app::MouseEvent &event)
{
    if (glm::length(position - ci::vec2(event.getPos())) < size)
    {
        if (event.isAltDown())
        {
            if (selection == Selection::addEdge)
            {
                selection = Selection::none;
            }
            else
            {
                selection = Selection::addEdge;
                selectedInFrame = ci::app::getElapsedFrames();
            }

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
    ci::gl::drawSolidCircle(position, size);
}


void GraphNodeHandler::draw(const ColorScheme &cs, bool highlighted)
{
    ci::Color color;
    if (highlighted)
    {
        color = cs.highlightedNodeColor1;
    }
    else
    {
        switch (selection)
        {
        case Selection::none:
            color = cs.nodeColor;
            break;
        case Selection::move:
            color = cs.movingNodeColor;
            break;
        case Selection::addEdge:
            color = cs.addEdgeNodeColor;
            break;
        default:
            color = cs.nodeColor;
            break;
        }
    }
    draw(color);
}

