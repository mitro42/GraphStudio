#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "Graph.h"
#include "GraphNodeHandler.h"

#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Fbo.h"

class GraphHandler
{
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

    void prepare(ci::app::WindowRef window);
    void update();
    void setup();
    void draw();
    void mouseDown(ci::app::MouseEvent &event);
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void toggleAutomaticEdgeWeightUpdate() { automaticEdgeWeightUpdate = !automaticEdgeWeightUpdate; }
    bool getAutomaticEdgeWeightUpdate() const { return automaticEdgeWeightUpdate; }
    void setAutomaticEdgeWeightUpdate(bool update) { automaticEdgeWeightUpdate = update; }

    void prepareAnimation();
    void generateGrid();
    void generateTriangleMesh();
private:
    enum Force{
        none = 0,
        push = 1,
        pull = 2
    };
    ci::app::WindowRef window;
    ci::signals::scoped_connection	cbMouseDown;
    ci::signals::scoped_connection	cbMouseDrag;
    ci::signals::scoped_connection	cbMouseUp;
    ci::Font font;
    ci::gl::TextureFontRef textureFont;
    ci::gl::Fbo fbo;
    ci::Area windowSize;
    std::recursive_mutex updateMutex;

    Graph g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    bool automaticEdgeWeightUpdate = false;
    bool changed = true;
    Force forceType;

    int animationState;
    int framesSpentInState;
    std::vector<std::pair<std::vector<std::pair<double, int>>, std::set<std::pair<double, int>>>> edgeWeightDijkstraStates;

    void drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle);
    void drawEdge(int from, int to, double weight = 0.0, bool highlight = false);
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawHighlightNodes();

    void drawAlgorithmState();

    void recreateNodeHandlers();
    void recreateNodeHandlers(const std::vector<ci::Vec2f> &nodePositions);
    void addNewEdgeIfNodesSelected();
    void updateEdgeWeights();
};

