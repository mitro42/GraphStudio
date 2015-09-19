#pragma once

#include "GraphAnimationDrawer.h"
#include "GraphNodeHandler.h"

class GraphHandler
{
public:
    enum class GraphType
    {
        general = 0,
        grid,
        triangleMesh
    };

public:
    GraphHandler();
    ~GraphHandler();
    
    void loadGraph(std::string fileName);
    void saveGraph(std::string fileName);
    void loadGraphPositions(std::string fileName);
    void saveGraphPositions(std::string fileName);

    void algorithmChanged();
    void algorithmStartNodeChanged();

    void reorderNodesSquare();
    void reorderNodesGrid(int columns, int rows);
    void pushNodes(ci::vec2 position, float force);
    void pushNodes(ci::vec2 position);
    void pullNodes(ci::vec2 position);

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
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void fitToWindow();

    void toggleAutomaticEdgeWeightUpdate() { automaticEdgeWeightUpdate = !automaticEdgeWeightUpdate; if (automaticEdgeWeightUpdate) g->setWeightedEdges(true); }
    bool getAutomaticEdgeWeightUpdate() const { return automaticEdgeWeightUpdate; }
    void setAutomaticEdgeWeightUpdate(bool update) { automaticEdgeWeightUpdate = update; }
    void setRandomEdgeWeights();
    void generateSpecialGraph(GraphType type);
private:
    enum class Force{
        none = 0,
        push = 1,
        pull = 2
    };
    
    ci::app::WindowRef window;
    ci::signals::ScopedConnection	cbMouseDown;
    ci::signals::ScopedConnection	cbMouseDrag;
    ci::signals::ScopedConnection	cbMouseUp;
    
    ci::Area windowSize;
    std::recursive_mutex updateMutex;

    std::shared_ptr<Graph> g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    std::unique_ptr<GraphAnimationDrawer> graphDrawer;

    bool automaticEdgeWeightUpdate = false;
    bool changed = true;
    Force forceType;

    void repositionNodes(const std::vector<ci::vec2>& nodePositions);
    void recreateNodeHandlers();
    void recreateNodeHandlers(const std::vector<ci::vec2> &nodePositions);
    void addNewEdgeIfNodesSelected();
    void updateEdgeWeights();
};

