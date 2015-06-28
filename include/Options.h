#ifndef GRAPHSTUDIO_OPTIONS_H
#define GRAPHSTUDIO_OPTIONS_H

#include "cinder\Color.h"

class Options
{
public:
    float nodeSize;
    float arrowLength;
    float arrowAngle;
    int speed;
    float edgeWidth;
    float highlighedEdgeWidth;
    bool animationPlaying;

    ci::Color backgroundColor;
    ci::Color nodeColor;
    ci::Color highlightedNodeColor;
    ci::Color movingNodeColor;
    ci::Color addEdgeNodeColor;

    ci::Color edgeColor;
    ci::Color highlightedEdgeColor;    
private:
    Options()
    {
        nodeSize = 10.0f;
        arrowLength = 15.0f;
        arrowAngle = 16.0f;
        speed = 60;
        edgeWidth = 1.5f;
        highlighedEdgeWidth = 3.0f;
        animationPlaying = false;
        backgroundColor = ci::Color(0.694f ,0.761f ,0.557f);
        nodeColor = ci::Color(0.176f, 0.133f, 0.310f);
        edgeColor = ci::Color(0.459f, 0.427f, 0.561f);
        highlightedNodeColor = ci::Color(0.984f, 0.961f, 0.831f);
        highlightedEdgeColor = ci::Color(0.361f, 0.129f, 0.259f);
        movingNodeColor = ci::Color("white");
        addEdgeNodeColor = ci::Color("yellow");
    }

public:
    ~Options() = default;

    static Options &instance() 
    { 
        static Options instance;
        return instance;
    }

};


class GridGraphParams
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
    GridGraphParams()
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
    ~GridGraphParams() = default;

    static GridGraphParams &instance()
    {
        static GridGraphParams instance;
        return instance;
    }
};

#endif // GRAPHSTUDIO_OPTIONS_H
