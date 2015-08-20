#include "stdafx.h"
#include "GraphAnimationDrawer.h"
#include "Options.h"

#include <string>
void GraphAnimationDrawer::prepareAnimation()
{
    animationMode = true;
    paused = false;
    animationState = 0;
    framesSpentInState = 0;
    createLegend();
}

void GraphAnimationDrawer::drawAnimationStateNumber()
{
    ci::gl::color(ci::ColorA(0.0f, 0.0f, 0.0f, 0.5f));
    std::string stateNumber = std::to_string(animationState + 1) + "/" + std::to_string(animationLastState);
    nodeTextureFont->drawString(stateNumber, ci::Vec2f(20.0f, window->getHeight() - 20.0f));
}

void GraphAnimationDrawer::draw()
{
    if (animationState < animationLastState - 1 && !paused)
    {        
        framesSpentInState++;
        if (framesSpentInState % Options::instance().speed == 0)
        {
            prepareNewState();
            animationState++;
        }
    }

    if (animationMode)
    {        
        drawAlgorithmState();
    }
    else
    {
        drawAlgorithmResult();
    }

    ci::gl::color(ci::Color::white());
    if (legendTexture = legend.getTexture())
    {
        ci::gl::draw(legendTexture, ci::Vec2f(float(window->getWidth() - legendTexture.getWidth()), 0));
    }

    drawAnimationStateNumber();
    clearChanged();
}

void GraphAnimationDrawer::nextState()
{
    if (animationState < animationLastState - 1)
    {
        animationState++;
        prepareNewState();
        paused = true;
    }
}

void GraphAnimationDrawer::previousState()
{
    if (animationState > 0)
    {
        animationState--;
        prepareNewState();
        paused = true;
    }
}
