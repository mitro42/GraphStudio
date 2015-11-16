#include "stdafx.h"
#include "Legend.h"

#include "GraphDrawer.h"

#include <cinder/gl/Fbo.h>

Legend::Legend(int width) : width(width), height(0)
{    
    format.setSamples(8);

    ci::Font font = ci::Font("InputMono Black", 20.0f);
    ci::gl::TextureFont::Format fontFormat;
    fontFormat.premultiply(true);    
    textureFont = ci::gl::TextureFont::create(font, fontFormat);    
};

ci::gl::TextureRef Legend::getTexture(bool forceRerender)
{ 
    if (!fbo && contents.empty())
    {
        return ci::gl::TextureRef();
    }
    else
    {        
        render(forceRerender);
        changed = false;
    }

    return fbo->getColorTexture(); 
}

void Legend::render(bool forceRerender)
{
    ci::vec2 currentSize;
    if (fbo)
    {
        currentSize = fbo->getSize();
    }
    const int rowHeight = 25;
    ci::vec2 requiredSize(width, float(contents.size() * rowHeight));
    if (requiredSize == currentSize && !forceRerender)
        return;
    
    fbo = ci::gl::Fbo::create(int(requiredSize.x), int(requiredSize.y), format);

    ci::gl::ScopedFramebuffer fbScp(fbo);
    ci::gl::ScopedViewport scpVp(ci::ivec2(0), fbo->getSize());
    
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow(fbo->getSize());
    ci::gl::clear(backgroundColor);
    
    float posY = rowHeight / 2;
    for (const auto &element: contents)
    {
        float edgeWidth = settings.edgeWidth;
        switch (element.type)        
        {
        case LegendType::highlightedArrow:
			edgeWidth = settings.highlightedEdgeWidth;
            // fall trough
        case LegendType::arrow:
            GraphDrawer::drawArrow(ci::vec2(5.0f, posY), ci::vec2(70.0f, posY), element.color, edgeWidth, settings.arrowLength, settings.arrowAngle);
            break;
        case LegendType::highlightedEdge:
			edgeWidth = settings.highlightedEdgeWidth;
            // fall trough
        case LegendType::edge:
            GraphDrawer::drawEdge(ci::vec2(5.0f, posY), ci::vec2(70.0f, posY), element.color, edgeWidth);
            break;
        case LegendType::node: 
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::vec2(40.0f, posY), settings.nodeSize);
            break;
        case LegendType::nodes:
            ci::gl::color(element.color);
            ci::gl::drawSolidCircle(ci::vec2(25.0f, posY), settings.nodeSize);
            ci::gl::drawSolidCircle(ci::vec2(55.0f, posY), settings.nodeSize);
            break;
        case LegendType::multiColorEdge:
        {
			edgeWidth = settings.highlightedEdgeWidth;
            std::vector<ci::Color> colors = { { 0.36f, 0.9f, 0.46f }, { 0.9f, 0.46f, 0.36f },  { 0.56f, 0.9f, 0.36f } };
            const float segmentLength = 65.0f / colors.size();
            for (int i = 0; i < colors.size(); ++i)
            {
                GraphDrawer::drawEdge(ci::vec2(5.0f + i * segmentLength, posY), ci::vec2(5.0f + (i + 1) * segmentLength, posY), colors[i], edgeWidth);
            }
            break;
        }
        default:
            break;
        }

        ci::gl::enableAlphaBlending(true);

        ci::gl::color(ci::ColorA(element.color, 1.0f));
        textureFont->drawString(element.label, ci::vec2(80, posY + rowHeight / 4));

        posY += rowHeight;
    }

    ci::gl::popMatrices();
}
