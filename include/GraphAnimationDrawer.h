#pragma once

#include "GraphDrawer.h"
#include "Legend.h"

class GraphAnimationDrawer : public GraphDrawer
{
public:
    GraphAnimationDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) : 
        GraphDrawer(graph, nodeHandlers, window), animationState(0), animationLastState(0)
    {
    }

    ~GraphAnimationDrawer() = default;

    virtual void draw();
    virtual void prepareAnimation();
    virtual void pause() { paused = true; }
    virtual void resume() { paused = false; }
    virtual void nextState();
    virtual void previousState();
protected:
    virtual void drawAlgorithmState() = 0;
    virtual void drawAlgorithmResult() = 0;
    virtual void prepareNewState() {}
    virtual void createLegend() = 0;

    int animationState;
    int animationLastState;
    Legend legend;
private:
    bool animationMode = true;
    bool paused = true;
    int framesSpentInState;
};