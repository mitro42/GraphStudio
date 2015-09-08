#pragma once
#include "stdafx.h"
#include "Graph.h"
#include "GraphNodeHandler.h"

#include <memory>

#include <cinder/gl/TextureFont.h>
#include <cinder/gl/Fbo.h>

class GraphDrawer
{
public:
    friend class Legend;
    GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window);
    virtual ~GraphDrawer() = default;

    //void update();
    void draw();
    void resize(ci::Area newWindowSize);
    void setChanged() { changed = true; }
protected:

    void drawEdge(int from, int to, ci::Color color, float width);

    static void drawArrow(ci::vec2 from, ci::vec2 to, float headLength, float headAngle);
    static void drawArrow(ci::vec2 from, ci::vec2 to, ci::Color color, float width);
    static void drawEdge(ci::vec2 from, ci::vec2 to, ci::Color color, float width);

    void startDrawing();
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawLabels(std::map<std::shared_ptr<GraphEdge>, ci::ColorA> &colors = std::map<std::shared_ptr<GraphEdge>, ci::ColorA>());
    void drawHighlightNodes();
    void drawColorScale();

    void clearChanged() { changed = false; }
    bool movingNodes() const;
        
    ci::app::WindowRef window;
    const std::shared_ptr<Graph> g;
    const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers;
    std::vector<ci::Color> generateColors(int n);

    ci::gl::TextureRef legendTexture;
    ci::gl::TextureFontRef edgeTextureFont;
    ci::gl::TextureFontRef nodeTextureFont;
private:
    bool changed = true;
    void initFbo();
    float oldNodeSize;
    
    ci::gl::FboRef edgeFbo;
    ci::gl::FboRef labelFbo;

    ci::Font edgeFont;
    ci::Font nodeFont;
    
    ci::TextBox getStateTextbox(const std::vector<std::string> &lines);
};

