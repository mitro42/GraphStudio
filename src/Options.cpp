#include "stdafx.h"
#include "Options.h"

std::vector<std::string> AlgorithmNames{
    "None",
    "Depth-first search",
    "Dijkstra's shortest paths",
    "Prim's MST",
    "Kruskal's MST"
};


ColorScheme::ColorScheme()
{
    name = "default";
    backgroundColor = ci::Color(0.694f, 0.761f, 0.557f);
    nodeColor = ci::Color(0.176f, 0.133f, 0.310f);
    edgeColor = ci::Color(0.459f, 0.427f, 0.561f);
    darkEdgeColor = ci::Color(0.359f, 0.327f, 0.401f);
    highlightedNodeColor1 = ci::Color("red");
    highlightedNodeColor2 = ci::Color(0.984f, 0.961f, 0.831f);
    highlightedNodeColor3 = ci::Color(0.984f, 0.961f, 0.831f);
    highlightedEdgeColor1 = ci::Color("red");
    highlightedEdgeColor2 = ci::Color(0.361f, 0.129f, 0.259f);
    highlightedEdgeColor3 = ci::Color(0.361f, 0.129f, 0.259f);

    nodeTextColor = ci::Color::white();
    highlightednodeTextColor = ci::Color::black();
    edgeTextColor = ci::Color::black();
    highlightedEdgeTextColor = ci::Color::black();
    
    movingNodeColor = ci::Color("white");
    addEdgeNodeColor = ci::Color("yellow");
}

ColorScheme ColorScheme::fromXml(ci::XmlTree &xml)
{
    ColorScheme cs;
    if (xml.getTag() != "colorScheme")
        throw "Invalid colorScheme tag";
    cs.name = xml.getAttribute("name").getValue();
    cs.backgroundColor = colorFromXml(xml.getChild("backgroundColor"));
    cs.nodeColor = colorFromXml(xml.getChild("nodeColor"));
    cs.edgeColor = colorFromXml(xml.getChild("edgeColor"));
    cs.darkEdgeColor = colorFromXml(xml.getChild("darkEdgeColor"));
    cs.highlightedNodeColor1 = colorFromXml(xml.getChild("highlightedNodeColor1"));
    cs.highlightedNodeColor2 = colorFromXml(xml.getChild("highlightedNodeColor2"));
    cs.highlightedNodeColor3 = colorFromXml(xml.getChild("highlightedNodeColor3"));
    cs.highlightedEdgeColor1 = colorFromXml(xml.getChild("highlightedEdgeColor1"));
    cs.highlightedEdgeColor2 = colorFromXml(xml.getChild("highlightedEdgeColor2"));
    cs.highlightedEdgeColor3 = colorFromXml(xml.getChild("highlightedEdgeColor3"));

    cs.nodeTextColor = colorFromXml(xml.getChild("nodeTextColor"));
    cs.highlightednodeTextColor = colorFromXml(xml.getChild("highlightednodeTextColor"));
    cs.edgeTextColor = colorFromXml(xml.getChild("edgeTextColor"));
    cs.highlightedEdgeTextColor = colorFromXml(xml.getChild("highlightedEdgeTextColor"));

    cs.movingNodeColor = colorFromXml(xml.getChild("movingNodeColor"));
    cs.addEdgeNodeColor = colorFromXml(xml.getChild("addEdgeNodeColor"));

    cs.legendBackgroundColor = colorFromXml(xml.getChild("legendBackgroundColor"));
    return cs;
}

ci::XmlTree ColorScheme::toXml() const
{
    ci::XmlTree xml("colorScheme", "");
    xml.setAttribute("name", name);
    xml.push_back(colorToXml("backgroundColor", backgroundColor));
    xml.push_back(colorToXml("nodeColor", nodeColor));
    xml.push_back(colorToXml("edgeColor", edgeColor));
    xml.push_back(colorToXml("darkEdgeColor", darkEdgeColor));
    xml.push_back(colorToXml("highlightedNodeColor1", highlightedNodeColor1));
    xml.push_back(colorToXml("highlightedNodeColor2", highlightedNodeColor2));
    xml.push_back(colorToXml("highlightedNodeColor3", highlightedNodeColor3));
    xml.push_back(colorToXml("highlightedEdgeColor1", highlightedEdgeColor1));
    xml.push_back(colorToXml("highlightedEdgeColor2", highlightedEdgeColor2));
    xml.push_back(colorToXml("highlightedEdgeColor3", highlightedEdgeColor3));

    xml.push_back(colorToXml("nodeTextColor", nodeTextColor));
    xml.push_back(colorToXml("highlightednodeTextColor", highlightednodeTextColor));
    xml.push_back(colorToXml("edgeTextColor", edgeTextColor));
    xml.push_back(colorToXml("highlightedEdgeTextColor", highlightedEdgeTextColor));

    xml.push_back(colorToXml("movingNodeColor", movingNodeColor));
    xml.push_back(colorToXml("addEdgeNodeColor", addEdgeNodeColor));

    xml.push_back(colorToXml("legendBackgroundColor", legendBackgroundColor));
    return xml;
}


ci::Color ColorScheme::colorFromXml(ci::XmlTree &xml)
{
    ci::Color c;
    c.r = xml.getAttribute("red").getValue<float>() / 255.0f;
    c.g = xml.getAttribute("green").getValue<float>() / 255.0f;
    c.b = xml.getAttribute("blue").getValue<float>() / 255.0f;
    return c;
}

ci::XmlTree ColorScheme::colorToXml(const char *name, const ci::Color &c)
{
    ci::XmlTree xml(name, "");
    xml.setAttribute("red", int(c.r * 255));
    xml.setAttribute("green", int(c.g * 255));
    xml.setAttribute("blue", int(c.b * 255));
    return xml;
}