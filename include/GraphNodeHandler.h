#pragma once
#include <cinder/App/MouseEvent.h>
#include "Options.h"
class GraphHandler;

class GraphNodeHandler
{
public:
    enum class Selection {
        none = 0,
        move,
        addEdge
    };

    GraphNodeHandler(ci::app::WindowRef window, GraphHandler &graphHandler, ci::vec2 pos);
    
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseDown(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void draw(const ci::Color &color);
    void draw(const ColorScheme &cs, bool highlighted = false);

    inline void update() 
    { 
        auto vec = ci::vec2(1.0f, 0.0f); 
        position += glm::rotate(vec, direction) * speed;
        
        position.x = std::max(margin, position.x);
        position.x = std::min(window->getWidth() - margin, position.x);

        position.y = std::max(margin, position.y);
        position.y = std::min(window->getHeight() - margin, position.y);
    }
    inline void clearSelection() { selection = Selection::none; }
    inline Selection getSelection() const { return selection; }

    inline ci::vec2 getPos() const { return position; }
    inline void setPos(const ci::vec2 &pos) { position = pos; }

    inline ci::vec2 getOriginalPos() const { return originalPosition; }

    inline float getDirection() { return direction; }
    inline void setDirection(const float &dir) { direction = dir; }

    inline float getSpeed() const { return speed; }
    inline void setSpeed(float sp) { speed = sp; }

    inline uint32_t getSelectedInFrame() const { return selectedInFrame; }
	static inline void setSize(float s) { size = s; }	

	void connectMouseEvents();
	void disconnectMouseEvents();
private:
    static const float margin;
	static float size;
    ci::signals::ScopedConnection cbMouseDrag;
    ci::signals::ScopedConnection cbMouseDown;
    ci::signals::ScopedConnection cbMouseUp;
    ci::app::WindowRef window;
    GraphHandler &graphHandler;
    ci::vec2 position;
    const ci::vec2 originalPosition;
    float direction;
    float speed;
	
    Selection selection;
    uint32_t selectedInFrame;
};

