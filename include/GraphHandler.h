#pragma once

#include "GraphAnimationDrawer.h"
#include "GraphNodeHandler.h"

struct GraphGenerator;

#include <cinder/Filesystem.h>

class GraphHandler
{
public:
	GraphHandler() = default;
	~GraphHandler() = default;
    
    void loadGraph(const ci::fs::path& fileName);
    void saveGraph(const ci::fs::path& fileName);
    void loadGraphPositions(const ci::fs::path& fileName);
    void saveGraphPositions(const ci::fs::path& fileName);

    void algorithmChanged(Algorithm newAlgorithm);
    void algorithmStartNodeChanged();

    void reorderNodesSquare();
    void reorderNodesGrid(int columns, int rows);

    void setup(ci::app::WindowRef window);
    void update();
    void draw();

    void setChanged() { changed = true; animationPrepare(); }

    void animationPrepare();
    void animationPause();
    void animationResume();
    bool animationNext(); // returns true if there are more states to play
    void animationPrevious();
    void animationGoToFirst();
    void animationGoToLast();

    void mouseDown(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void fitToWindow();

	void setRandomEdgeWeights(int min, int max);
    void generateSpecialGraph(const GraphGenerator& generator);
    GraphAnimationDrawer& getAnimationDrawer() { return *graphDrawer; }

	inline void startRandomMovement() { randomMovement = true; }
	inline void stopRandomMovement() { randomMovement = false; }

	void setEdgeWeightsFromLengths(double scale);
	void connectMouseEvents();
	void disconnectMouseEvents();
private:
	void repositionNodes(const std::vector<ci::vec2>& newNodePositions);
	void recreateNodeHandlers();
	void recreateNodeHandlers(const std::vector<ci::vec2> &newNodePositions);
	void addNewEdgeIfNodesSelected();	
	bool nodeHandlersChanged();
	void clearChanged();
	void addNodeHandler(ci::vec2 pos);

    ci::app::WindowRef window;
    ci::signals::ScopedConnection	cbMouseDown;
    ci::signals::ScopedConnection	cbMouseDrag;
    ci::signals::ScopedConnection	cbMouseUp;
    
    ci::Area windowSize;
    std::recursive_mutex updateMutex;

    std::shared_ptr<Graph> g;
	std::vector<std::unique_ptr<ci::vec2>> nodePositions;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    std::unique_ptr<GraphAnimationDrawer> graphDrawer;

    bool changed = true;
	bool randomMovement = false;
};

