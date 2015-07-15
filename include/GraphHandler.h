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
    enum GraphType
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

    void prepare(ci::app::WindowRef window);
    void update();
    void setup();
    void draw();
    void mouseDown(ci::app::MouseEvent &event);
    void mouseDrag(ci::app::MouseEvent &event);
    void mouseUp(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void fitToWindow();

    void toggleAutomaticEdgeWeightUpdate() { automaticEdgeWeightUpdate = !automaticEdgeWeightUpdate; if (automaticEdgeWeightUpdate) g.setWeightedEdges(true); }
    bool getAutomaticEdgeWeightUpdate() const { return automaticEdgeWeightUpdate; }
    void setAutomaticEdgeWeightUpdate(bool update) { automaticEdgeWeightUpdate = update; }

    void prepareAnimation();
    void generateSpecialGraph(GraphType type);
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
    float oldNodeSize;

    Graph g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    bool automaticEdgeWeightUpdate = false;
    bool changed = true;
    Force forceType;

    int animationState;
    int animationLastState;
    int framesSpentInState;
    std::vector<graph_algorithm_capture::ShortestPathEdgeWeightDijkstraState> edgeWeightDijkstraStates;
    std::vector<graph_algorithm_capture::MstPrimState> mstPrimStates;
    std::vector<graph_algorithm_capture::MstKruskalState> mstKruskalStates;
    std::vector<ci::Color> algorithmColorScale;

    void drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle);
    void drawEdge(int from, int to, ci::Color color, float width = 1.0f);
    void drawEdge(int from, int to, bool highlight = false);
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawLabels();
    void drawHighlightNodes();
    void drawColorScale();

    void drawAlgorithmStateDijkstra();
    void drawAlgorithmStateMstPrim();
    void drawAlgorithmStateMstKruskal();

    std::vector<ci::Color> generateColors(int n);

    void repositionNodes(const std::vector<ci::Vec2f>& nodePositions);
    void recreateNodeHandlers();
    void recreateNodeHandlers(const std::vector<ci::Vec2f> &nodePositions);
    void addNewEdgeIfNodesSelected();
    void updateEdgeWeights();
};

