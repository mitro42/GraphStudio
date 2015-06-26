#pragma once
#include <memory>
#include <vector>
#include "Graph.h"
#include "GraphNodeHandler.h"

#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"

class GraphHandler
{
public:
    GraphHandler();
    ~GraphHandler();
    
    void loadGraph(std::string fileName);
    void saveGraph(std::string fileName);
    void loadGraphPositions(std::string fileName);
    void saveGraphPositions(std::string fileName);

    void prepare(ci::app::WindowRef window);
    void update();
    void setup();
    void draw();
    void mouseDown(ci::app::MouseEvent &event);
    
private:
    ci::app::WindowRef window;
    ci::signals::scoped_connection	cbMouseDown;
    ci::Font font;
    ci::gl::TextureFontRef textureFont;


    Graph g;
    std::vector<std::unique_ptr<GraphNodeHandler>> nodeHandlers;
    
    float nodeRadius;

    void drawEdge(int from, int to, double weight = 0.0, bool highlight = false);
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawHighlightNodes();

    void recreateNodeHandlers();
};

