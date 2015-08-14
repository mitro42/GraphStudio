#include "stdafx.h"
#include "GraphDrawer.h"
#include "Options.h"

GraphDrawer::GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
    g(graph),
    nodeHandlers(nodeHandlers),
    window(window)
{
    initFbo();

    legendFont = ci::Font("InputMono Black", 15);
    legendTextureFont = ci::gl::TextureFont::create(legendFont);
}

void GraphDrawer::resize(ci::Area newWindowSize)
{
    initFbo();
}

void GraphDrawer::initFbo()
{
    ci::gl::Fbo::Format format;
    //format.enableColorBuffer();
    format.setSamples(8);
    //format.enableMipmapping();
    fbo = ci::gl::Fbo(window->getWidth(), window->getHeight(), format);
}

void GraphDrawer::startDrawing()
{
    ci::gl::enableAlphaBlending();
    ci::gl::clear(Options::instance().currentColorScheme.backgroundColor);
}


void GraphDrawer::draw()
{
    //if (!changed)
    //    return;
    //fbo.bindFramebuffer();
    startDrawing();

    drawEdges();
    drawHighlightEdges();
    drawNodes();
    drawLabels();
    
    //fbo.unbindFramebuffer();
    //ci::gl::draw(fbo.getTexture());
}



void GraphDrawer::drawEdges()
{
    for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
    {
        auto &node = g->getNode(nodeIdx);
        for (auto edgePtr : node)
        {
            drawEdge(edgePtr->from,  edgePtr->to, false);
        }
    }
}

void GraphDrawer::drawArrow(ci::Vec2f from, ci::Vec2f to, float headLength, float headAngle)
{
    ci::gl::drawLine(from, to);
    ci::Vec2f dir = (to - from).normalized();
    dir.rotate(ci::toRadians(headAngle));
    ci::gl::drawLine(to, to - dir*headLength);
    dir.rotate(ci::toRadians(-2 * headAngle));
    ci::gl::drawLine(to, to - dir*headLength);
}

void GraphDrawer::drawArrow(ci::Vec2f from, ci::Vec2f to, ci::Color color, float width)
{
    ci::gl::color(color);
    ci::gl::lineWidth(width);
    drawArrow(from, to, Options::instance().arrowLength, Options::instance().arrowAngle);
}

void GraphDrawer::drawEdge(ci::Vec2f from, ci::Vec2f to, ci::Color color, float width)
{
    ci::gl::lineWidth(width);
    ci::gl::color(color);
    ci::gl::drawLine(from, to);
}

void GraphDrawer::drawEdge(int from, int to, ci::Color color, float width)
{
    if (from == to)
        return;

    ci::gl::lineWidth(width);
    ci::gl::color(color);

    // calculating the end points and drawing the lines/arrows
    ci::Vec2f fromVec = nodeHandlers[from]->getPos();
    ci::Vec2f toVec = nodeHandlers[to]->getPos();
    if (g->isDirected())
    {
        ci::Vec2f dir = toVec - fromVec;
        dir.normalize();
        drawArrow(fromVec, toVec - Options::instance().nodeSize * dir,
            Options::instance().arrowLength, Options::instance().arrowAngle);
    }
    else
    {
        ci::gl::drawLine(fromVec, toVec);
    }
}

void GraphDrawer::drawEdge(int from, int to, bool highlight)
{
    fbo.bindFramebuffer();
    ci::Area viewPort = ci::gl::getViewport();
    ci::gl::setViewport(fbo.getBounds());
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow(fbo.getSize(), false);
    ci::gl::clear(ci::ColorA(0, 0, 0, 0.0f));

    float width = Options::instance().edgeWidth;
    ci::Color color = Options::instance().currentColorScheme.edgeColor;
    if (highlight)
    {
        width = Options::instance().highlighedEdgeWidth;
        color = Options::instance().currentColorScheme.highlightedEdgeColor2;
    }

    drawEdge(from, to, color, width);

    ci::gl::popMatrices();
    ci::gl::setViewport(viewPort);
    fbo.unbindFramebuffer();
    ci::gl::draw(fbo.getTexture());
}


void GraphDrawer::drawHighlightEdges()
{
    if (Options::instance().startNode > g->getNodeCount())
    {
        Options::instance().startNode = g->getNodeCount() - 1;
    }
    if (Options::instance().startNode < 0 && g->getNodeCount() > 0)
    {
        Options::instance().startNode = 1;
    }
}


void GraphDrawer::drawNodes()
{
    for (auto &nh : nodeHandlers)
    {
        nh->draw();
    }
}

void GraphDrawer::drawLabels()
{


    const auto &cs = Options::instance().currentColorScheme;
    if (oldNodeSize != Options::instance().nodeSize)
    {
        nodeFont = ci::Font("InputMono Black", Options::instance().nodeSize * 1.6f);
        edgeFont = nodeFont;
        nodeTextureFont = ci::gl::TextureFont::create(nodeFont);
        edgeTextureFont = nodeTextureFont;
        oldNodeSize = Options::instance().nodeSize;
    }

    // Nodes
    if (Options::instance().showNodeWeights)
    {
        for (int i = 0; i < g->getNodeCount(); ++i)
        {
            ci::gl::color(cs.nodeTextColor);
            auto label = std::to_string(i + 1);
            auto labelOffset = nodeTextureFont->measureString(label) / 2;
            labelOffset.y *= 0.65f;
            labelOffset.x *= -1;
            nodeTextureFont->drawString(label, nodeHandlers[i]->getPos() + labelOffset);
        }
    }

    // Edges
    if (Options::instance().showEdgeWeights && g->hasWeightedEdges())
    {
        for (auto &node: *g)
        {
            for (auto &edgePtr : *node)
            {
                // writing edge weight
                ci::Vec2f fromVec = nodeHandlers[edgePtr->from]->getPos();
                ci::Vec2f toVec = nodeHandlers[edgePtr->to]->getPos();

                std::stringstream ss;
                ss << std::fixed << std::setprecision(Options::instance().weightPrecision) << edgePtr->weight;
                std::string labelText = ss.str();
                ci::Vec2f textMid = (fromVec + toVec) / 2;
                ci::Vec2f edgeDir = fromVec - toVec;
                float deg = ci::toDegrees(std::atan(edgeDir.y / edgeDir.x));
                auto textRect = edgeTextureFont->measureString(labelText);
                float textWidth = textRect.x;
                ci::Vec2f textAlignmentOffset = edgeDir.normalized() * textWidth / 2.0f;
                if (fromVec.x < toVec.x)
                {
                    textAlignmentOffset *= -1;
                }
                ci::gl::pushModelView();
                ci::gl::enableAlphaBlending();
                ci::gl::translate(textMid - textAlignmentOffset);
                if (edgeDir.x != 0)
                {
                    ci::gl::rotate(deg);
                }
                else
                {
                    ci::gl::rotate(90);
                }
                ci::Vec2f offset = -(fromVec + toVec).normalized() * 10; // place edge weight over the edge
                ci::gl::color(cs.edgeTextColor);
                edgeTextureFont->drawString(labelText, offset);
                ci::gl::popModelView();
            }
        }
    }


   
}

void GraphDrawer::drawHighlightNodes()
{
}


std::vector<ci::Color> GraphDrawer::generateColors(int n)
{
    std::vector<ci::Color> ret;
    ret.reserve(n);

    for (int i = 0; i < n; ++i)
    {
        ci::Color c(ci::ColorModel::CM_HSV, float(i) / (n + 1), 0.6f, 0.9f);
        ret.push_back(c);
    }
    return ret;
}
