#pragma once
#include <memory>
#include "cinder/app/Window.h"
#include "cinder/Color.h"
#include "cinder/Font.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Vector.h"

#include "Graph.h"
#include "GraphNodeHandler.h"

class GraphDrawer
{
public:
    GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers) : g(graph), nodeHandlers(nodeHandlers)
    {
    }

    ~GraphDrawer() = default;

    void update();
    void setup(ci::app::WindowRef _window);
    void draw();

    
protected:

    void drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle);
    void drawEdge(int from, int to, ci::Color color, float width = 1.0f);
    void drawEdge(int from, int to, bool highlight = false);

    void startDrawing();
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawLabels();
    void drawHighlightNodes();
    void drawColorScale();

    ci::app::WindowRef window;
    const std::shared_ptr<Graph> g;
    const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers;
    std::vector<ci::Color> generateColors(int n);
    ci::gl::TextureFontRef legendTextureFont;
    ci::Font legendFont;
    ci::gl::Texture legendTexture;
private:
    float oldNodeSize;
    
    ci::gl::Fbo fbo;

    ci::Font edgeFont;
    ci::Font nodeFont;
    
    ci::gl::TextureFontRef edgeTextureFont;
    ci::gl::TextureFontRef nodeTextureFont;

    
    ci::TextBox getStateTextbox(const std::vector<std::string> &lines);
};

