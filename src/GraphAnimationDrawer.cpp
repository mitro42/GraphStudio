#include "stdafx.h"
#include "GraphAnimationDrawer.h"
#include "Options.h"

#include <string>
void GraphAnimationDrawer::prepareAnimation()
{
    animationMode = true;
    paused = true;
    animationGoToLast();
    framesSpentInState = 0;
    createLegend();
}

void GraphAnimationDrawer::drawAnimationStateNumber()
{
    ci::gl::color(ci::ColorA(0.0f, 0.0f, 0.0f, 0.5f));
    std::string stateNumber = std::to_string(animationState + 1) + "/" + std::to_string(animationLastState);
    stepDescriptionTextureFont->drawString(stateNumber, ci::vec2(20.0f, window->getHeight() - 20.0f));
}

void GraphAnimationDrawer::draw()
{
    if (animationState == animationLastState - 1)
    {
        Options::instance().animationPlaying = false;
        Options::instance().animationPaused = true;
    }
    else if (animationState < animationLastState - 1 && !paused)
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
        ci::gl::draw(legendTexture, ci::vec2(float(window->getWidth() - legendTexture->getWidth()), float(window->getHeight() - legendTexture->getHeight())));
    }

    drawAnimationStateNumber();
    clearChanged();
}

bool GraphAnimationDrawer::nextState()
{
    if (animationState < animationLastState - 1)
    {
        animationState++;
        prepareNewState();
        paused = true;
        return true;
    }
    return false;    
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


void GraphAnimationDrawer::drawStepDescription(const std::string& description)
{
    ci::gl::color(ci::ColorA(0.0f, 0.0f, 0.0f, 0.7f));
    auto size = stepDescriptionTextureFont->measureString(description);
    stepDescriptionTextureFont->drawString(description, ci::vec2((window->getWidth() - size.x) / 2.0f, size.y * 1.5f));
}