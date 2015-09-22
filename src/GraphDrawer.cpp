#include "stdafx.h"
#include "GraphDrawer.h"
#include "Options.h"

#include <algorithm>
#include <numeric>

GraphDrawer::GraphDrawer(std::shared_ptr<Graph> graph, const std::vector<std::unique_ptr<GraphNodeHandler>> &nodeHandlers, ci::app::WindowRef window) :
    g(graph),
    nodeHandlers(nodeHandlers),
    window(window)
{
    initFbo();
}

void GraphDrawer::resize(ci::Area newWindowSize)
{
    initFbo();
}

void GraphDrawer::initFbo()
{
    auto format = ci::gl::Fbo::Format().samples(8);    
    
    edgeFbo = ci::gl::Fbo::create(window->getWidth(), window->getHeight(), format);
    labelFbo = ci::gl::Fbo::create(window->getWidth(), window->getHeight(), format);
}

void GraphDrawer::startDrawing()
{
    ci::gl::enableAlphaBlending();
    ci::gl::clear(Options::instance().currentColorScheme.backgroundColor);

    nodeFont = ci::Font("InputMono Black", Options::instance().nodeSize * 1.6f);
    edgeFont = nodeFont;
    nodeTextureFont = ci::gl::TextureFont::create(nodeFont);
    edgeTextureFont = nodeTextureFont;
}


bool GraphDrawer::movingNodes() const
{
    return std::accumulate(begin(nodeHandlers), end(nodeHandlers), false,
        [](bool lhs, const std::unique_ptr<GraphNodeHandler>& rhs) 
            { return lhs || (rhs->getSelection() == GraphNodeHandler::Selection::move); }
    );
}

void GraphDrawer::draw()
{
    startDrawing();
    drawEdges();
    drawHighlightEdges();
    drawNodes();
    drawLabels();
    clearChanged();
}



void GraphDrawer::drawEdges()
{
    if (changed)
    {
        //ci::app::console() << "Redrawing edges" << std::endl;
        ci::gl::ScopedFramebuffer scpFbEdge(edgeFbo);
        ci::gl::ScopedViewport scpVpEdge(edgeFbo->getSize());
        
        ci::gl::pushMatrices();
        ci::gl::setMatricesWindow(edgeFbo->getSize());
        ci::gl::clear(ci::ColorA(0, 0, 0, 0.0f));

        for (int nodeIdx = 0; nodeIdx < g->getNodeCount(); ++nodeIdx)
        {
            auto &node = g->getNode(nodeIdx);
            for (auto edgePtr : node)
            {
                drawEdge(edgePtr->from, edgePtr->to, Options::instance().currentColorScheme.edgeColor, Options::instance().edgeWidth);
            }
        }
        ci::gl::popMatrices();
    }
    ci::gl::draw(edgeFbo->getColorTexture());
}

void GraphDrawer::drawArrow(ci::vec2 from, ci::vec2 to, float headLength, float headAngle)
{
    ci::vec2 dir = glm::normalize(to - from);
    ci::vec2 lineEnd = to - dir * headLength / 2.0f;
    ci::gl::drawLine(from, lineEnd);
    
    auto rot = glm::rotate(dir, ci::toRadians(headAngle));
    ci::gl::drawSolidTriangle(to, to - rot*headLength, lineEnd);
    rot = glm::rotate(dir, ci::toRadians(-headAngle));
    ci::gl::drawSolidTriangle(to, to - rot*headLength, lineEnd);    
}

void GraphDrawer::drawArrow(ci::vec2 from, ci::vec2 to, ci::Color color, float width)
{
    ci::gl::color(color);
    ci::gl::lineWidth(width);
    drawArrow(from, to, Options::instance().arrowLength, Options::instance().arrowAngle);
}

void GraphDrawer::drawEdge(ci::vec2 from, ci::vec2 to, ci::Color color, float width)
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
    ci::vec2 fromVec = nodeHandlers[from]->getPos();
    ci::vec2 toVec = nodeHandlers[to]->getPos();
    if (g->isDirected())
    {
        ci::vec2 dir = toVec - fromVec;        
        drawArrow(fromVec, toVec - Options::instance().nodeSize * glm::normalize(dir),
            Options::instance().arrowLength, Options::instance().arrowAngle);
    }
    else
    {
        ci::gl::drawLine(fromVec, toVec);
    }
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

void GraphDrawer::drawLabels(std::map<std::shared_ptr<GraphEdge>, ci::ColorA> &colors)
{
    if (changed)
    {
        //ci::app::console() << "Redrawing labels" << std::endl;
        ci::gl::ScopedFramebuffer scpFbLabel(labelFbo);
        ci::gl::ScopedViewport scpVpLabel(ci::ivec2(0), labelFbo->getSize());
        
        
        ci::gl::pushMatrices();
        ci::gl::setMatricesWindow(labelFbo->getSize());
        ci::gl::clear(ci::ColorA(Options::instance().currentColorScheme.backgroundColor, 0.0f));

        const auto &cs = Options::instance().currentColorScheme;

        // Nodes
        if (Options::instance().showNodeWeights)
        {
            for (int i = 0; i < g->getNodeCount(); ++i)
            {
                ci::gl::color(cs.nodeTextColor);
                auto label = std::to_string(i + 1);
                auto labelOffset = nodeTextureFont->measureString(label) / 2.0f;
                labelOffset.y *= 0.68f;
                labelOffset.x *= -1;
                nodeTextureFont->drawString(label, nodeHandlers[i]->getPos() + labelOffset);
            }
        }

        // Edges
        if (Options::instance().showEdgeWeights && g->hasWeightedEdges())
        {
            for (auto &node : *g)
            {
                for (auto &edgePtr : *node)
                {
                    // writing edge weight
                    ci::vec2 fromVec = nodeHandlers[edgePtr->from]->getPos();
                    ci::vec2 toVec = nodeHandlers[edgePtr->to]->getPos();

                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(Options::instance().weightPrecision) << edgePtr->weight;
                    std::string labelText = ss.str();
                    ci::vec2 textMid = (fromVec + toVec) / 2.0f;
                    ci::vec2 edgeDir = fromVec - toVec;
                    float rad = std::atan(edgeDir.y / edgeDir.x);
                    auto textRect = edgeTextureFont->measureString(labelText);
                    float textWidth = textRect.x;
                    ci::vec2 textAlignmentOffset = glm::normalize(edgeDir) * textWidth / 2.0f;
                    if (fromVec.x < toVec.x)
                    {
                        textAlignmentOffset *= -1;
                    }
                    ci::gl::pushModelView();
                    ci::gl::translate(textMid - textAlignmentOffset);
                    if (edgeDir.x != 0)
                    {
                        ci::gl::rotate(rad);
                    }
                    else
                    {
                        ci::gl::rotate(glm::half_pi<float>());
                    }
                    ci::vec2 offset = -ci::vec2(0.0f, 5.0f + Options::instance().highlighedEdgeWidth); // place edge weight over the edge
                    ci::ColorA c = cs.edgeTextColor;
                    auto it = colors.find(edgePtr);
                    if (it != colors.end())
                        c = it->second;
                    ci::gl::color(c);
                    edgeTextureFont->drawString(labelText, offset);
                    ci::gl::popModelView();
                }
            }
        }

        ci::gl::popMatrices();
    }
    ci::gl::color(ci::Color::white());
    ci::gl::draw(labelFbo->getColorTexture());
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
