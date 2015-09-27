#ifndef GRAPHSTUDIO_OPTIONS_H
#define GRAPHSTUDIO_OPTIONS_H

#include "cinder/Color.h"
#include "cinder/Xml.h"
#include <string>
#include <vector>

extern std::vector<std::string> AlgorithmNames;

enum class Algorithm {
    none = 0,
    dfs,
    dijkstra,
    prim,
    kruskal
};

struct ColorScheme
{
    std::string name;
    ci::Color backgroundColor;
    ci::Color nodeColor;
    ci::Color highlightedNodeColor1;
    ci::Color highlightedNodeColor2;
    ci::Color highlightedNodeColor3;
    ci::Color movingNodeColor;
    ci::Color addEdgeNodeColor;

    ci::Color edgeColor;
    ci::Color darkEdgeColor;
    ci::Color highlightedEdgeColor1;
    ci::Color highlightedEdgeColor2;
    ci::Color highlightedEdgeColor3;

    ci::Color nodeTextColor;
    ci::Color highlightednodeTextColor;
    ci::Color edgeTextColor;
    ci::Color highlightedEdgeTextColor;

    ci::Color legendBackgroundColor;
    ColorScheme();
    static ColorScheme fromXml(ci::XmlTree &xml);
    ci::XmlTree toXml() const;

private:
    static ci::Color colorFromXml(ci::XmlTree &xml);
   static ci::XmlTree colorToXml(const char *name, const ci::Color &c);
};

class Options
{
public:
    float nodeSize;
    float arrowLength;
    float arrowAngle;
    int speed;
    float edgeWidth;
    float highlightedEdgeWidth;
    float force;
    bool animationPlaying;
    bool animationPaused;
    bool randomMovement;
    bool showEdgeWeights;
    bool showNodeWeights;
    int weightPrecision;
    int edgeWeightScale;
    float minRandomEdgeWeight;
    float maxRandomEdgeWeight;
    int algorithm;
    int startNode;
    ColorScheme currentColorScheme;
    int currentColorSchemeIdx;

    float legendEdgeWidth;
    float legendHighlightedEdgeWidth;
    float legendArrowLength;
    float legendArrowAngle;
    float legendNodeSize;

private:
    Options()
    {
        nodeSize = 10.0f;
        arrowLength = 15.0f;
        arrowAngle = 16.0f;
        speed = 60;
        edgeWidth = 1.5f;
        highlightedEdgeWidth = 3.0f;
        force = 10.0;
        edgeWeightScale = 100;
        int weightPrecision = 0;
        showEdgeWeights = true;
        showNodeWeights = true;
        animationPlaying = false;
        animationPaused = false;
        algorithm = static_cast<int>(Algorithm::dijkstra);
        randomMovement = false;
        startNode = 1;
        minRandomEdgeWeight = 0.0f;
        maxRandomEdgeWeight = 100.0f;

        legendEdgeWidth = edgeWidth;
        legendHighlightedEdgeWidth = highlightedEdgeWidth;
        legendArrowLength = arrowLength;
        legendArrowAngle = arrowAngle;
        legendNodeSize = nodeSize;
    }

public:
    ~Options() = default;

    static Options &instance()
    {
        static Options instance;
        return instance;
    }
};



class GraphParamsGrid
{
public:
    int columns;
    int rows;
    bool directed;
    bool horizontal;
    bool vertical;
    bool upDiagonal;
    bool downDiagonal;
private:
    GraphParamsGrid()
    {
        columns = 3;
        rows = 3;
        directed = false;
        horizontal = true;
        vertical = true;
        upDiagonal = true;
        downDiagonal = true;
    }
public:
    ~GraphParamsGrid() = default;

    static GraphParamsGrid &instance()
    {
        static GraphParamsGrid instance;
        return instance;
    }
};


class GraphParamsTriangleMesh
{
public:
    int triangles;
    float randomness;
private:
    GraphParamsTriangleMesh()
    {
        triangles = 10;
        randomness = 0.0f;
    }
public:
    ~GraphParamsTriangleMesh() = default;

    static GraphParamsTriangleMesh &instance()
    {
        static GraphParamsTriangleMesh instance;
        return instance;
    }
};

#endif // GRAPHSTUDIO_OPTIONS_H
