#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Graph.h"
#include "GraphAnimationDrawer.h"
#include "GraphNodeHandler.h"

#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Fbo.h"

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

    void reorderNodesSquare();
    void reorderNodesGrid(int columns, int rows);
    void pushNodes(ci::Vec2f position, float force);
    void pushNodes(ci::Vec2f position);
    void pullNodes(ci::Vec2f position);

    void setup(ci::app::WindowRef window);
    void update();
    void draw();
    void prepareAnimation();
    void mouseDown(ci::app::MouseEvent &event);
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void fitToWindow();

    void toggleAutomaticEdgeWeightUpdate() { automaticEdgeWeightUpdate = !automaticEdgeWeightUpdate; if (automaticEdgeWeightUpdate) g->setWeightedEdges(true); }
    bool getAutomaticEdgeWeightUpdate() const { return automaticEdgeWeightUpdate; }
    void setAutomaticEdgeWeightUpdate(bool update) { automaticEdgeWeightUpdate = update; }
    
    void generateSpecialGraph(GraphType type);
private:
    enum class Force{
        none = 0,
        push = 1,
        pull = 2
    };
    
    ci::app::WindowRef window;
    ci::signals::scoped_connection	cbMouseDown;
    ci::signals::scoped_connection	cbMouseDrag;
    ci::signals::scoped_connection	cbMouseUp;
    ci::gl::Fbo fbo;
    ci::Area windowSize;
    std::recursive_mutex updateMutex;

    std::shared_ptr<Graph> g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    std::unique_ptr<GraphAnimationDrawer> graphDrawer;

    bool algorithmAnimationMode = false;
    bool algorithmAnimationPlaying = false;

    bool automaticEdgeWeightUpdate = false;
    bool changed = true;
    Force forceType;
    
    void repositionNodes(const std::vector<ci::Vec2f>& nodePositions);
    void recreateNodeHandlers();
    void recreateNodeHandlers(const std::vector<ci::Vec2f> &nodePositions);
    void addNewEdgeIfNodesSelected();
    void updateEdgeWeights();
};

