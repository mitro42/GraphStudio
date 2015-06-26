#pragma once
#include <memory>
#include <vector>
#include "Graph.h"
#include "GraphNodeHandler.h"

#include "cinder/Vector.h"

class GraphHandler
{
public:
    GraphHandler();
    ~GraphHandler();
    
    void prepare(ci::app::WindowRef window);
    void update();
    void setup();
    void draw();
    void mouseDown(ci::app::MouseEvent &event);
    
private:
    ci::app::WindowRef window;
    ci::signals::scoped_connection	cbMouseDown;

    Graph g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    std::vector<std::pair<int, int>> edges;
    float nodeRadius;

    void drawEdge(int from, int to, bool highlight);
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawHighlightNodes();
};

