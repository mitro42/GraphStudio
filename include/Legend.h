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
    nodes
};

struct LegendItem
{
    LegendType type;
    ci::Color color;
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

    ci::gl::Texture getTexture();

private:
    void render();

    bool changed = true;

    int width;
    int height;    
    ci::gl::Fbo::Format format;
    ci::gl::TextureFontRef textureFont;

    std::vector<LegendItem> contents;
    ci::gl::Fbo  fbo;
};