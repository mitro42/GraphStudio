#pragma once

#include "Options.h"

#include <vector>

#include <cinder/Color.h>
#include <cinder/gl/Texture.h>


enum class LegendType
{
    highlightedEdge,
    highlightedArrow,
    edge,
    arrow,
    node,
    nodes,
    multiColorEdge
};

struct LegendItem
{
    LegendType type;
    ci::ColorA color;
    std::string label;
    LegendItem(LegendType type, ci::Color color, const std::string &label) : type(type), color(color), label(label) {}
};

class Legend
{
public:
	Legend();
    ~Legend() = default;

    void clear() { contents.clear(); }

    void add(LegendType type, ci::Color color, const std::string &label)
    {
        contents.emplace_back(type, color, label);
        changed = true;
    }
	void setBackgroundColor(const ci::Color &color) { backgroundColor = color; }
	void setDrawingSettings(const GraphDrawingSettings &s) { settings = s; }
    ci::gl::TextureRef getTexture(bool forceRerender = false);

private:
    void render(bool forceRerender = false);

    bool changed = true;

    int width;
    int height;    
    ci::gl::Fbo::Format format;
    ci::gl::TextureFontRef textureFont;
	ci::Color backgroundColor;

    std::vector<LegendItem> contents;
    ci::gl::FboRef  fbo;
	GraphDrawingSettings settings;
};