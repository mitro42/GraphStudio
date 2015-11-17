#pragma once

#include "GraphDrawer.h"
#include "Legend.h"
#include "Options.h"
#include <cinder/Font.h>

class GraphAnimationDrawer : public GraphDrawer
{
public:
    GraphAnimationDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize, int infoPanelWidth = 350) : 
        GraphDrawer(graph, nodeHandlers, windowSize), 
        animationState(-1), 
        animationLastState(-1),
		legend(infoPanelWidth)
    {
        ci::Font stepFont = ci::Font("InputMono Black", 36);
        stepDescriptionTextureFont = ci::gl::TextureFont::create(stepFont);
    }

    virtual ~GraphAnimationDrawer() = default;

    virtual void draw();
    virtual void prepareAnimation(int startNode);
    virtual void pause() { playingState = AnimationPlayingState::pause; }
    virtual void resume() { playingState = AnimationPlayingState::play; }
    virtual bool nextState(); // returns true if there are more states to play
    virtual void previousState();
    virtual void animationGoToFirst() { animationState = 0; }
    virtual void animationGoToLast() = 0;    

    void showAnimationStateNumber(bool show = true) { animationStateNumberVisible = show; }
    bool getShowAnimationStateNumber() const { return animationStateNumberVisible; }
    void showAnimationStateDescription(bool show = true) { animationStateDescriptionVisible = show; }
    bool getShowAnimationStateDescription() const { return animationStateDescriptionVisible; }
    void showLegend(bool show = true) { legendVisible = show; }
    bool getShowLegend() const { return legendVisible; }
	int getFramesPerState() const { return framesPerState; }
	void setFramesPerState(int frames) { framesPerState = frames; }
    int getAnimationStateNumber() const { return animationState; }
	virtual void setColorScheme(const ColorScheme &cs);
	virtual void setDrawingSettings(const GraphDrawingSettings &settings)
	{
		GraphDrawer::setDrawingSettings(settings); 
		legend.setDrawingSettings(settings);
	}
	bool isAnimationFinished();
protected:
    virtual void drawAlgorithmState() = 0;
    //virtual void drawAlgorithmResult(int startNode) = 0;
    virtual void prepareNewState() {}
    virtual void createLegend() = 0;
    virtual void drawAnimationStateNumber();
    virtual void drawStepDescription(const std::string& description);

	int animationState = -1;
	int animationLastState = -1;
    bool animationStateDescriptionVisible = true;
    bool animationStateNumberVisible = true;
    bool legendVisible = true;
    Legend legend;
private:
	AnimationPlayingState playingState = AnimationPlayingState::stop;
	int framesSpentInState = 0;
	int framesPerState = 60;
    ci::gl::TextureFontRef stepDescriptionTextureFont;
    ci::gl::TextureRef legendTexture;
};