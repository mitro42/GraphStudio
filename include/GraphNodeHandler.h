#pragma once
#include <cinder/App/MouseEvent.h>

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

    inline void update() 
    { 
        auto vec = ci::Vec2f(1.0f, 0.0f); 
        vec.rotate(direction); 
        position += vec * speed; 
        
        position.x = std::max(margin, position.x);
        position.x = std::min(window->getWidth() - margin, position.x);

        position.y = std::max(margin, position.y);
        position.y = std::min(window->getHeight() - margin, position.y);
    }
    inline void clearSelection() { selection = Selection::none; }
    inline Selection getSelection() const { return selection; }

    inline ci::Vec2f getPos() const { return position; }
    inline void setPos(const ci::Vec2f &pos) { position = pos; }

    inline ci::Vec2f getOriginalPos() const { return originalPosition; }

    inline float getDirection() { return direction; }
    inline void setDirection(const float &dir) { direction = dir; }

    inline float getSpeed() const { return speed; }
    inline void setSpeed(float sp) { speed = sp; }

private:
    static const float margin;
    ci::signals::scoped_connection cbMouseDrag;
    ci::signals::scoped_connection cbMouseDown;
    ci::signals::scoped_connection cbMouseUp;
    ci::app::WindowRef window;

    ci::Vec2f position;
    const ci::Vec2f originalPosition;
    float direction;
    float speed;
    Selection selection;
};

