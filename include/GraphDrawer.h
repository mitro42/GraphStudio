#pragma once
#include "stdafx.h"
#include "Graph.h"
#include "GraphNodeHandler.h"

#include <cinder/gl/TextureFont.h>
#include <cinder/gl/Fbo.h>

class GraphDrawer
{
public:
    friend class Legend;
    GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window);
    ~GraphDrawer() = default;

    //void update();
    void draw();
    void resize(ci::Area newWindowSize);
    void setChanged() { changed = true; }
protected:

    void drawEdge(int from, int to, ci::Color color, float width = 1.0f);
    void drawEdge(int from, int to, bool highlight = false);

    static void drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle);
    static void drawArrow(ci::Vec2f from, ci::Vec2f to, ci::Color color, float width);
    static void drawEdge(ci::Vec2f from, ci::Vec2f to, ci::Color color, float width);

    void startDrawing();
    void drawEdges();
    void drawHighlightEdges();
    void drawNodes();
    void drawLabels();
    void drawHighlightNodes();
    void drawColorScale();

    void clearChanged() { changed = false; }
    bool movingNodes() const;
        
    ci::app::WindowRef window;
    const std::shared_ptr<Graph> g;
    const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers;
    std::vector<ci::Color> generateColors(int n);
    ci::gl::TextureFontRef legendTextureFont;
    ci::Font legendFont;
    ci::gl::Texture legendTexture;
private:
    bool changed = true;
    void initFbo();
    float oldNodeSize;
    
    ci::gl::Fbo edgeFbo;
    ci::gl::Fbo labelFbo;

    ci::Font edgeFont;
    ci::Font nodeFont;
    
    ci::gl::TextureFontRef edgeTextureFont;
    ci::gl::TextureFontRef nodeTextureFont;

    
    ci::TextBox getStateTextbox(const std::vector<std::string> &lines);
};

