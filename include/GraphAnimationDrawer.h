#pragma once

#include "GraphDrawer.h"
#include "Legend.h"
#include <cinder/Font.h>

class GraphAnimationDrawer : public GraphDrawer
{
public:
    GraphAnimationDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) : 
        GraphDrawer(graph, nodeHandlers, window), 
        animationState(-1), 
        animationLastState(-1)
    {
        ci::Font stepFont = ci::Font("InputMono Black", 36);
        stepDescriptionTextureFont = ci::gl::TextureFont::create(stepFont);
    }

    virtual ~GraphAnimationDrawer() = default;

    virtual void draw();
    virtual void prepareAnimation();
    virtual void pause() { paused = true; }
    virtual void resume() { paused = false; }
    virtual bool nextState(); // returns true if there are more states to play
    virtual void previousState();
    virtual void animationGoToFirst() { animationState = 0; }
    virtual void animationGoToLast() = 0;
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