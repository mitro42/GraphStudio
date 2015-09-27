#include "stdafx.h"
#include "Legend.h"

#include "GraphDrawer.h"

#include <cinder/gl/Fbo.h>

Legend::Legend() : width(0), height(0)
{    
    format.setSamples(8);
    ci::Font font = ci::Font("InputMono Black", 20.0f);
    textureFont = ci::gl::TextureFont::create(font);
};

ci::gl::TextureRef Legend::getTexture()
{ 
    if (!fbo && contents.empty())
    {
        return ci::gl::TextureRef();
    }
    else
    {        
        render();
        changed = false;
    }

    return fbo->getColorTexture(); 
}

void Legend::render()
{
    ci::vec2 currentSize;
    if (fbo)
    {
        currentSize = fbo->getSize();
    }
    const int rowHeight = 25;
    ci::vec2 requiredSize(350.0f, float(contents.size() * rowHeight));
    if (requiredSize != currentSize)
    {
        fbo = ci::gl::Fbo::create(int(requiredSize.x), int(requiredSize.y), format);
    }
    else
        return;
    
    ci::gl::ScopedFramebuffer fbScp(fbo);
    ci::gl::ScopedViewport scpVp(ci::ivec2(0), fbo->getSize());
    
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow(fbo->getSize());
    ci::gl::clear(Options::instance().currentColorScheme.legendBackgroundColor);
    
    float posY = rowHeight / 2;
    for (const auto &element: contents)
    {
        float width = Options::instance().legendEdgeWidth;
        switch (element.type)        
        {
        case LegendType::highlightedArrow:
            width = Options::instance().legendHighlightedEdgeWidth;
            // fall trough
        case LegendType::arrow:
            GraphDrawer::drawArrow(ci::vec2(5.0f, posY), ci::vec2(70.0f, posY), element.color, width, Options::instance().legendArrowLength, Options::instance().legendArrowAngle);
            break;
        case LegendType::highlightedEdge:
            width = Options::instance().legendHighlightedEdgeWidth;
            // fall trough
        case LegendType::edge:
            GraphDrawer::drawEdge(ci::vec2(5.0f, posY), ci::vec2(70.0f, posY), element.color, width);
            break;
        case LegendType::node: 
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::vec2(40.0f, posY), Options::instance().legendNodeSize);
            break;
        case LegendType::nodes:
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::vec2(25.0f, posY), Options::instance().legendNodeSize);
            ci::gl::drawSolidCircle(ci::vec2(55.0f, posY), Options::instance().legendNodeSize);
            break;
        default:
            break;
        }

        textureFont->drawString(element.label, ci::vec2(80, posY + rowHeight / 4));

        posY += rowHeight;
    }

    ci::gl::popMatrices();
}
