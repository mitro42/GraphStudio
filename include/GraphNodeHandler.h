#pragma once
#include "cinder/App/App.h"

class GraphNodeHandler
{
public:
    enum Selection {
        none = 0,
        move,
        addEdge
    };

    GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos);

    void mouseDrag(ci::app::MouseEvent &event);
    void mouseDown(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void draw();

    void clearSelection() { selection = none; }
    Selection getSelection() const { return selection; }
    ci::Vec2f getPos() const { return rect.getCenter(); }
    void setPos(ci::Vec2f pos) { rect.offsetCenterTo(pos); }

private:
    ci::signals::scoped_connection cbMouseDrag;
    ci::signals::scoped_connection cbMouseDown;
    ci::signals::scoped_connection cbMouseUp;
    ci::app::WindowRef window;

    ci::Rectf rect;
    Selection selection;
};

