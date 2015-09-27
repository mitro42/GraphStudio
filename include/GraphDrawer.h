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
    virtual void colorSchemeChanged() { setChanged(); }
protected:
    struct EdgeDrawParams
    {
        ci::Color color;
        float width;
        EdgeDrawParams() :
            color(ci::Color::black()),
            width(5.0)
        {}

        EdgeDrawParams(const ci::Color &color, float width) :
            color(color),
            width(width)
        {}
    };

    typedef std::map<const GraphEdge*, EdgeDrawParams> EdgeDrawParamsMap;

    void drawEdge(int from, int to, ci::Color color, float width);

    static void drawArrow(ci::vec2 from, ci::vec2 to, float headLength, float headAngle);
    static void drawArrow(ci::vec2 from, ci::vec2 to, ci::Color color, float width, float headLength, float headAngle);
    static void drawArrow(ci::vec2 from, ci::vec2 to, ci::Color color, float width);
    static void drawEdge(ci::vec2 from, ci::vec2 to, ci::Color color, float width);

    void startDrawing();
    void drawEdges();
    void drawEdges(const EdgeDrawParamsMap &edges);
    void drawHighlightEdges();
    void drawNodes();
    void drawLabels(EdgeDrawParamsMap &params = EdgeDrawParamsMap());
    void drawHighlightNodes();
    void drawColorScale();

    EdgeDrawParamsMap createDefaultEdgeParams() const;

    void clearChanged() { changed = false; }
    bool movingNodes() const;
        
    ci::app::WindowRef window;
    const std::shared_ptr<Graph> g;
    const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers;
    std::vector<ci::Color> generateColors(int n);

    ci::gl::TextureFontRef edgeTextureFont;
    ci::gl::TextureFontRef nodeTextureFont;
private:
    bool changed = true;
    void initFbo();
    
    ci::gl::FboRef edgeFbo;
    ci::gl::FboRef labelFbo;

    ci::Font edgeFont;
    ci::Font nodeFont;
    
    ci::TextBox getStateTextbox(const std::vector<std::string> &lines);
};

