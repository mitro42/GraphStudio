#pragma once
#include "stdafx.h"
#include "Graph.h"
#include "GraphNodeHandler.h"
#include "Options.h"
#include <memory>

#include <cinder/gl/TextureFont.h>
#include <cinder/gl/Fbo.h>

class GraphDrawer
{
public:
    friend class Legend;
    GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::Area windowSize);
    virtual ~GraphDrawer() = default;

    void draw();
    void resize(ci::Area newWindowSize);
    void setChanged() { changed = true; }
	inline void setColorScheme(const ColorScheme &cs) { colorScheme = cs; setChanged(); }
	inline const ColorScheme &getColorScheme() const { return colorScheme; }

	virtual void setDrawingSettings(const GraphDrawingSettings &sett) { settings = sett; setChanged(); }
	inline const GraphDrawingSettings &getDrawingSettings() const { return settings; }

	inline bool getShowEdgeWeights() const { return showEdgeWeights; }
	inline void setShowEdgeWeights(bool show) { showEdgeWeights = show; setChanged(); }

	inline bool getShowNodeWeights() const { return showNodeWeights; }
	inline void setShowNodeWeights(bool show) { showNodeWeights = show; setChanged(); }

	inline int getWeightPrecision() const { return weightPrecision; }
	inline void setWeightPrecision(int precision) { weightPrecision = precision; }
protected:
    struct EdgeDrawParams
    {
        ci::Color color = ci::Color::black();
        float width = 5.0f;
        EdgeDrawParams() = default;
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
	
	ci::Area windowSize;
    const std::shared_ptr<Graph> g;
    const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers;
    std::vector<ci::Color> generateColors(int n);
	std::vector<ci::Color> generateColors(int n, ci::Color baseColor);

    ci::gl::TextureFontRef edgeTextureFont;
    ci::gl::TextureFontRef nodeTextureFont;
private:
	static GraphDrawingSettings settings;

    bool changed = true;
	bool showEdgeWeights = true;
	bool showNodeWeights = true;
    void initFbo(ci::Area newWindowSize);
    ci::TextBox getStateTextbox(const std::vector<std::string> &lines);

	ColorScheme colorScheme;
    ci::gl::FboRef edgeFbo;
    ci::gl::FboRef labelFbo;

    ci::Font edgeFont;
    ci::Font nodeFont;
	int weightPrecision = 0; // number of decimal digits when displaying weights
};

