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

enum class AnimationPlayingState
{
	stop,
	play,
	pause,
	recording
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

struct GraphDrawingSettings
{
	float nodeSize = 10.0f;
	float arrowLength = 15.0f;
	float arrowAngle = 16.0f;
	float edgeWidth = 1.5f;
	float highlightedEdgeWidth = 3.0f;
};

struct Options
{
    int speed = 60;
    
    int weightPrecision = 0;
    int edgeWeightScale = 100;
    
    int startNode = 1;
    bool autoFitToScreen = true;

    float infoPanelWidth = 350.0f;

    static Options &instance()
    {
        static Options instance;
        return instance;
    }
};



#endif // GRAPHSTUDIO_OPTIONS_H
