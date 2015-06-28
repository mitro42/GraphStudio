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

    void prepare(ci::app::WindowRef window);
    void update();
    void setup();
    void draw();
    void mouseDown(ci::app::MouseEvent &event);
    void resize(ci::Area newWindowSize);

    void toggleAutomaticEdgeWeightUpdate() { automaticEdgeWeightUpdate = !automaticEdgeWeightUpdate; }
    bool getAutomaticEdgeWeightUpdate() const { return automaticEdgeWeightUpdate; }
    void setAutomaticEdgeWeightUpdate(bool update) { automaticEdgeWeightUpdate = update; }

private:
    ci::app::WindowRef window;
    ci::signals::scoped_connection	cbMouseDown;
    ci::Font font;
    ci::gl::TextureFontRef textureFont;
    ci::gl::Fbo fbo;
    ci::Area windowSize;
    std::recursive_mutex updateMutex;

    Graph g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    bool automaticEdgeWeightUpdate = false;
    bool changed = true;

    void drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle);
    void drawEdge(int from, int to, double weight = 0.0, bool highlight = false);
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawHighlightNodes();

    void recreateNodeHandlers();
    void addNewEdgeIfNodesSelected();
    void updateEdgeWeights();
};

