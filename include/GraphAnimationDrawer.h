#pragma once

#include "GraphDrawer.h"
#include "Legend.h"
#include <cinder/Font.h>

class GraphAnimationDrawer : public GraphDrawer
{
public:
    GraphAnimationDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) : 
        GraphDrawer(graph, nodeHandlers, window), 
        animationState(0), 
        animationLastState(0)
    {
        ci::Font stepFont = ci::Font("InputMono Black", 36);
        stepDescriptionTextureFont = ci::gl::TextureFont::create(stepFont);
    }

    virtual ~GraphAnimationDrawer() = default;

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
    virtual void drawAnimationStateNumber();
    virtual void drawStepDescription(const std::string& description);

    int animationState;
    int animationLastState;
    Legend legend;
private:
    bool animationMode = false;
    bool paused = true;
    int framesSpentInState;
    ci::gl::TextureFontRef stepDescriptionTextureFont;
};