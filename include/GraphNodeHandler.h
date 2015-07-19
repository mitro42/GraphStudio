#pragma once
#include "cinder/App/App.h"

class GraphNodeHandler
{
public:
    enum class Selection {
        none = 0,
        move,
        addEdge
    };

    GraphNodeHandler(ci::app::WindowRef window, ci::Vec2f pos);
    
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseDown(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void draw(const ci::Color &color);
    void draw(bool highlighted = false);

    inline void update() { position += speed; }
    inline void clearSelection() { selection = Selection::none; }
    inline Selection getSelection() const { return selection; }
    inline ci::Vec2f getPos() const { return position; }
    inline void setPos(const ci::Vec2f &pos) { position = pos; }
    inline void setSpeed(const ci::Vec2f &sp) { speed = sp; }

private:
    ci::signals::scoped_connection cbMouseDrag;
    ci::signals::scoped_connection cbMouseDown;
    ci::signals::scoped_connection cbMouseUp;
    ci::app::WindowRef window;

    ci::Vec2f position;
    ci::Vec2f speed;
    Selection selection;
};

