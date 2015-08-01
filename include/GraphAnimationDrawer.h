#pragma once

#include "GraphDrawer.h"

class GraphAnimationDrawer : public GraphDrawer
{
public:
    GraphAnimationDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers) : GraphDrawer(graph, nodeHandlers)
    {
    }

    ~GraphAnimationDrawer() = default;

    virtual void draw();
    virtual void prepareAnimation();
protected:
    virtual void drawAlgorithmState() = 0;
    virtual void prepare() {}
    virtual void pause() { paused = true; }
    virtual void resume() { paused = false; }
    virtual void prepareNewState() {}

    int animationState;
    int animationLastState;
private:
    bool paused = true;
    int framesSpentInState;
};