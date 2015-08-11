#include "stdafx.h"
#include "GraphAnimationDrawer.h"
#include "Options.h"

void GraphAnimationDrawer::prepareAnimation()
{
    animationState = 0;
    framesSpentInState = 0;
    createLegend();
}

void GraphAnimationDrawer::draw()
{
    if (animationState < animationLastState - 1)
    {
        framesSpentInState++;
        if (framesSpentInState % Options::instance().speed == 0)
        {
            prepareNewState();
            animationState++;
        }
    }
    if (Options::instance().animationPlaying)
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
}