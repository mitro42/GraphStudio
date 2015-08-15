#include "stdafx.h"
#include "Legend.h"

#include "GraphDrawer.h"

#include <cinder/gl/Fbo.h>

Legend::Legend() : width(0), height(0)
{    
    format.setSamples(8);
    ci::Font font = ci::Font("InputMono", 20.0f);
    textureFont = ci::gl::TextureFont::create(font);
};

ci::gl::Texture Legend::getTexture()
{ 
    if (!fbo && contents.empty())
    {
        return ci::gl::Texture();
    }
    else 
    {
        render();
        changed = false;
    }

    return fbo.getTexture(); 
}

void Legend::render()
{
    ci::Vec2f currentSize;
    if (fbo)
    {
        currentSize = fbo.getSize();
    }
    const int rowHeight = 25;
    ci::Vec2f requiredSize(300.0f, float(contents.size() * rowHeight));
    if (requiredSize != currentSize)
    {
        fbo = ci::gl::Fbo(int(requiredSize.x), int(requiredSize.y), format);
    }
    
    ci::gl::SaveFramebufferBinding bindingSaver;
    fbo.bindFramebuffer();
    ci::Area viewPort = ci::gl::getViewport();
    ci::gl::setViewport(fbo.getBounds());
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow(fbo.getSize(), false);
    ci::gl::clear(ci::ColorA("black", 0.1f));
    
    float posY = rowHeight / 2;
    for (const auto &element: contents)
    {
        float width = Options::instance().edgeWidth;
        switch (element.type)        
        {
        case LegendType::highlightedArrow:
            width = Options::instance().highlighedEdgeWidth;
            // fall trough
        case LegendType::arrow:
            GraphDrawer::drawArrow(ci::Vec2f(5.0f, posY), ci::Vec2f(70.0f, posY), element.color, width);
            break;
        case LegendType::highlightedEdge:
            width = Options::instance().highlighedEdgeWidth;
            // fall trough
        case LegendType::edge:
            GraphDrawer::drawEdge(ci::Vec2f(5.0f, posY), ci::Vec2f(70.0f, posY), element.color, width);
            break;
        case LegendType::node: 
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::Vec2f(40.0f, posY), 10.0f);
            break;
        case LegendType::nodes:
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::Vec2f(25.0f, posY), 10.0f);
            ci::gl::drawSolidCircle(ci::Vec2f(55.0f, posY), 10.0f);
            break;
        default:
            break;
        }

        textureFont->drawString(element.label, ci::Vec2f(80, posY + rowHeight / 4));

        posY += rowHeight;
    }

    ci::gl::popMatrices();
    ci::gl::setViewport(viewPort);
}
