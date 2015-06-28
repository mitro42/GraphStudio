#ifndef GRAPHSTUDIO_OPTIONS_H
#define GRAPHSTUDIO_OPTIONS_H

#include "cinder\Color.h"

class Options
{
public:
    float nodeSize;
    float arrowLength;
    float arrowAngle;
    float speed;
    ci::Color nodeColor;
    ci::Color highlightedNodeColor;
    ci::Color movingNodeColor;
    ci::Color addEdgeNodeColor;

    ci::Color edgeColor;
    ci::Color highlightedEdgeColor;    
    
    Options()
    {
        nodeSize = 5.0f;
        arrowLength = 10.0f;
        arrowAngle = 30.0f;
        speed = 1.0f;
        nodeColor = ci::Color("red");
        edgeColor = ci::Color("green");
        highlightedNodeColor = ci::Color("darkgreen");
        highlightedEdgeColor = ci::Color("orange");
        movingNodeColor = ci::Color("white");
        addEdgeNodeColor = ci::Color("yellow");
    }


    ~Options() = default;

    static Options &instance() 
    { 
        static Options instance;
        return instance;
    }

};
#endif // GRAPHSTUDIO_OPTIONS_H
