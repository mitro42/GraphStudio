#include "stdafx.h"
#include "GraphAnimationDrawer.h"
#include "Options.h"

#include <string>
void GraphAnimationDrawer::prepareAnimation(int startNode)
{
	playingState = AnimationPlayingState::pause;
    animationGoToLast();
    framesSpentInState = 0;
    createLegend();
}

void GraphAnimationDrawer::setColorScheme(const ColorScheme & cs)
{
	GraphDrawer::setColorScheme(cs);
	setChanged();
	createLegend();
	legendTexture = legend.getTexture(true);
}

void GraphAnimationDrawer::drawAnimationStateNumber()
{
    if (!animationStateNumberVisible)
        return;

    ci::gl::color(ci::ColorA(0.0f, 0.0f, 0.0f, 0.5f));
    std::string stateNumber = std::to_string(animationState + 1) + "/" + std::to_string(animationLastState);
    stepDescriptionTextureFont->drawString(stateNumber, ci::vec2(20.0f, windowSize.getHeight() - 20.0f));
}

bool GraphAnimationDrawer::isAnimationFinished()
{
	return animationState == animationLastState - 1;
}

void GraphAnimationDrawer::draw()
{
    if (animationState < animationLastState - 1 && playingState != AnimationPlayingState::pause)
    {        
        framesSpentInState++;
        if (framesSpentInState % framesPerState == 0)
        {
            prepareNewState();
            animationState++;
        }
    }

    drawAlgorithmState();

    ci::gl::color(ci::Color::white());
    if (legendVisible && (legendTexture = legend.getTexture()))
    {
        ci::gl::draw(legendTexture, ci::vec2(float(windowSize.getWidth() - legendTexture->getWidth()), 
			float(windowSize.getHeight() - legendTexture->getHeight())));
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
		pause();
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
		pause();
    }
}


void GraphAnimationDrawer::drawStepDescription(const std::string& description)
{
    if (!animationStateDescriptionVisible)
        return;

    ci::gl::color(ci::ColorA(0.0f, 0.0f, 0.0f, 0.7f));
    auto size = stepDescriptionTextureFont->measureString(description);
    stepDescriptionTextureFont->drawString(description, ci::vec2((windowSize.getWidth() - size.x) / 2.0f, size.y * 1.5f));
}