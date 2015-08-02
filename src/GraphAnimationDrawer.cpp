#include "stdafx.h"
#include "GraphAnimationDrawer.h"
#include "Options.h"

void GraphAnimationDrawer::prepareAnimation()
{
    animationState = 0;
    framesSpentInState = 0;
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
}