#include "stdafx.h"

#include "GraphHandler.h"
#include "Options.h"

#include <cinder/app/AppNative.h>
#include <cinder/params/Params.h>
#include <cinder/ImageIo.h>

using namespace ci;
using namespace ci::app;

class GraphStudioApp : public AppNative {
public:
    void prepareSettings(Settings *settings) override;
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;
    void resize() override;
    void shutdown() override;

    const ColorScheme &getCurrentColorScheme();
private:
    ci::params::InterfaceGlRef	params;
    std::map<std::string, ColorScheme> colorSchemes;
    std::vector<std::string> colorSchemeNames;
    GraphHandler gh;
    bool recording = false;
    int prevColorSchemeIndex;
    void addNewColorScheme();
    void storeColorScheme();
    void prepareRecording();
    void stopRecording();
    void loadSettings();
    void saveSettings();
};


void GraphStudioApp::prepareSettings(Settings *settings)
{
    settings->enableConsoleWindow();
    settings->setWindowSize(800, 600);
    settings->setFrameRate(30.0f);
}


const ColorScheme &GraphStudioApp::getCurrentColorScheme()
{
    return colorSchemes[colorSchemeNames[Options::instance().currentColorSchemeIdx]];
}


void GraphStudioApp::addNewColorScheme()
{
    auto &cs = Options::instance().currentColorScheme;
    cs = ColorScheme();
    bool freeNameFound = false;
    int freeNameIdx = 0;
    std::string newName;
    do
    {
        newName = "ColorScheme #" + std::to_string(++freeNameIdx);
        if (find(begin(colorSchemeNames), end(colorSchemeNames), newName) == colorSchemeNames.end())
            freeNameFound = true;
    } while (!freeNameFound);
    cs.name = newName;
    colorSchemeNames.push_back(cs.name);
    colorSchemes[cs.name] = cs;
    Options::instance().currentColorSchemeIdx = int(colorSchemes.size() - 1);
    params->addParam("ColorScheme", colorSchemeNames, &Options::instance().currentColorSchemeIdx);
}


void GraphStudioApp::storeColorScheme()
{
    auto &cs = Options::instance().currentColorScheme;
    colorSchemes[cs.name] = cs;
}


void GraphStudioApp::saveSettings()
{
    ci::XmlTree configXml("graphStudioSettings", "");
    configXml.push_back(ci::XmlTree("nodeSize", std::to_string(Options::instance().nodeSize)));
    configXml.push_back(ci::XmlTree("edgeWidth", std::to_string(Options::instance().edgeWidth)));
    configXml.push_back(ci::XmlTree("highlightedEdgeWidth", std::to_string(Options::instance().highlighedEdgeWidth)));
    configXml.push_back(ci::XmlTree("arrowLength", std::to_string(Options::instance().arrowLength)));
    configXml.push_back(ci::XmlTree("arrowAngle", std::to_string(Options::instance().arrowAngle)));
    configXml.push_back(ci::XmlTree("showEdgeWeights", std::to_string(Options::instance().showEdgeWeights)));
    configXml.push_back(ci::XmlTree("showNodeWeights", std::to_string(Options::instance().showNodeWeights)));
    configXml.push_back(ci::XmlTree("force", std::to_string(Options::instance().force)));
    configXml.push_back(ci::XmlTree("speed", std::to_string(Options::instance().speed)));
    configXml.push_back(ci::XmlTree("edgeWeightScale", std::to_string(Options::instance().edgeWeightScale)));
    ci::XmlTree csList("colorSchemes", "");
    for (const auto &cs : colorSchemes)
    {
        csList.push_back(cs.second.toXml());
    }
    configXml.push_back(csList);

    configXml.write(ci::writeFile("config.xml"));
}


void GraphStudioApp::loadSettings()
{
    ci::XmlTree configXml(ci::loadFile("config.xml"));
    ci::XmlTree settings = configXml.getChild("graphStudioSettings");
    Options::instance().nodeSize = settings.getChild("nodeSize").getValue<float>();
    Options::instance().edgeWidth = settings.getChild("edgeWidth").getValue<float>();
    Options::instance().highlighedEdgeWidth = settings.getChild("highlightedEdgeWidth").getValue<float>();
    Options::instance().arrowLength = settings.getChild("arrowLength").getValue<float>();
    Options::instance().arrowAngle = settings.getChild("arrowAngle").getValue<float>();
    Options::instance().showEdgeWeights = settings.getChild("showEdgeWeights").getValue<bool>();
    Options::instance().showNodeWeights = settings.getChild("showNodeWeights").getValue<bool>();

    Options::instance().force = settings.getChild("force").getValue<float>();
    Options::instance().speed = settings.getChild("speed").getValue<int>();
    Options::instance().edgeWeightScale = settings.getChild("edgeWeightScale").getValue<int>();

    ci::XmlTree csList = settings.getChild("colorSchemes");
    for (auto csIt = csList.begin(); csIt != csList.end(); ++csIt)
    {
        ColorScheme cs = ColorScheme::fromXml(*csIt);
        colorSchemes[cs.name] = cs;
        colorSchemeNames.push_back(cs.name);
    }
}


void GraphStudioApp::setup()
{    
    loadSettings();

    params = params::InterfaceGl::create("Graph Studio", Vec2i(200, 310));
    params->addParam("Node Size", &Options::instance().nodeSize, "min=1.0 max=50.0 step=1.0");
    params->addParam("Edge Width", &Options::instance().edgeWidth, "min=0.1 max=10.0 step=0.1");
    params->addParam("Highlighted Edge Width", &Options::instance().highlighedEdgeWidth, "min=0.0 max=10.0 step=0.1");
    params->addParam("Arrow Length", &Options::instance().arrowLength, "min=1.0 max=50.0 step=1.0");
    params->addParam("Arrow Angle", &Options::instance().arrowAngle, "min=0.0 max=90.0 step=1.0");
    params->addParam("Show Edge Weights", &Options::instance().showEdgeWeights);
    params->addParam("Show Node Weights", &Options::instance().showNodeWeights);
    params->addSeparator();
    params->addParam("Algorithm", AlgorithmNames, &Options::instance().algorithm);
    params->addParam("Starting Node",  &Options::instance().startNode, "min=1 step=1");
    params->addSeparator();
    params->addParam("Force", &Options::instance().force, "min=1.0 max=300.0 step=1.0");
    params->addParam("Speed", &Options::instance().speed, "min=1.0 max=300.0 step=1.0");
    params->addParam("Edge Weight Scale", &Options::instance().edgeWeightScale, "min=1.0 max=1000.0 step=1.0");
    params->addSeparator();
    params->addText("Colors");
    params->addParam("ColorScheme", colorSchemeNames, &Options::instance().currentColorSchemeIdx);
    params->addParam("Background", &Options::instance().currentColorScheme.backgroundColor);

    params->addParam("Node ", &Options::instance().currentColorScheme.nodeColor);
    params->addParam("Highlighted Node 1", &Options::instance().currentColorScheme.highlightedNodeColor1);
    params->addParam("Highlighted Node 2", &Options::instance().currentColorScheme.highlightedNodeColor2);
    params->addParam("Highlighted Node 3", &Options::instance().currentColorScheme.highlightedNodeColor3);

    params->addParam("Edge", &Options::instance().currentColorScheme.edgeColor);
    params->addParam("Dark Edge", &Options::instance().currentColorScheme.darkEdgeColor);
    params->addParam("Highlighted Edge 1", &Options::instance().currentColorScheme.highlightedEdgeColor1);
    params->addParam("Highlighted Edge 2", &Options::instance().currentColorScheme.highlightedEdgeColor2);
    params->addParam("Highlighted Edge 3", &Options::instance().currentColorScheme.highlightedEdgeColor3);

    params->addParam("Node Text", &Options::instance().currentColorScheme.nodeTextColor);
    params->addParam("highlightednodeText", &Options::instance().currentColorScheme.highlightednodeTextColor);
    params->addParam("Edge Text", &Options::instance().currentColorScheme.edgeTextColor);
    params->addParam("Highlighted Edge Text", &Options::instance().currentColorScheme.highlightedEdgeTextColor);

    params->addButton("New", std::bind(&GraphStudioApp::addNewColorScheme, this));
    params->addSeparator();
    params->addText("Random Edge Weights");
    params->addParam("Min", &Options::instance().minRandomEdgeWeight, "min=1, max=1000, step=1");
    params->addParam("Max", &Options::instance().maxRandomEdgeWeight, "min=1, max=1000, step=1");
    params->addButton("Generate Weights", std::bind(&GraphHandler::setRandomEdgeWeights, &gh));
    params->addSeparator();
    params->addText("Generate Grid");
    params->addParam("Columns", &GraphParamsGrid::instance().columns, "min=1 step=1");
    params->addParam("Rows", &GraphParamsGrid::instance().rows, "min=1 step=1");
    params->addParam("Directed", &GraphParamsGrid::instance().directed);
    params->addParam("Horizontal Edges", &GraphParamsGrid::instance().horizontal);
    params->addParam("Vertical Edges", &GraphParamsGrid::instance().vertical);
    params->addParam("Diagonal /", &GraphParamsGrid::instance().upDiagonal);
    params->addParam("Diagonal \\", &GraphParamsGrid::instance().downDiagonal);
    params->addButton("Generate grid", std::bind(&GraphHandler::generateSpecialGraph, &gh, GraphHandler::GraphType::grid));
    params->addText("Generate Triangle Mesh");
    params->addParam("Triangles", &GraphParamsTriangleMesh::instance().triangles, "min=1 step=1");
    params->addParam("Randomness", &GraphParamsTriangleMesh::instance().randomness, "min=0.0 step=0.1");
    params->addButton("Generate tri", std::bind(&GraphHandler::generateSpecialGraph, &gh, GraphHandler::GraphType::triangleMesh));
    
    prevColorSchemeIndex = -1;
    gh.setup(getWindow());

}

void GraphStudioApp::shutdown()
{
    saveSettings();
}

void GraphStudioApp::stopRecording()
{
    recording = false;
    Options::instance().animationPlaying = false;
    Options::instance().animationPaused = true;
    gh.animationPause();
    setFullScreen(false);
    //showCursor();
    params->show();
    setFrameRate(30.0f);
}


void GraphStudioApp::prepareRecording()
{
    setFrameRate(30.0f);
    recording = true;
    setFullScreen(true);
    params->hide();
    Options::instance().animationPlaying = true;
    Options::instance().animationPaused = false;
    gh.animationPrepare();
    gh.animationPause();
}


void GraphStudioApp::keyDown(KeyEvent event)
{
    std::string nameBase = "graph_small2";
    static const std::vector<std::string> extensions{ "graph", "txt" };
    if (event.getCode() == KeyEvent::KEY_SPACE)
    {
        // stop - play - pause - play - pause - play - until the last state is reached
        if (!Options::instance().animationPlaying)
        {
            Options::instance().animationPlaying = true;
            Options::instance().animationPaused = false;
            gh.animationPrepare();
        }
        else
        {
            Options::instance().animationPaused = !Options::instance().animationPaused;
            if (Options::instance().animationPaused)
            {
                gh.animationPause();
            }
            else
            {
                gh.animationResume();
            }
        }
        
        return;
    }
    else  if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        gh.animationNext();
        return;
    }
    else  if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        gh.animationPrevious();
        return;
    }
    else  if (event.getCode() == KeyEvent::KEY_ESCAPE)
    {
        stopRecording();
        return;
    }
    if (event.getChar() == 'm')
    {
        Options::instance().randomMovement = !Options::instance().randomMovement;
        if (Options::instance().randomMovement)
            Options::instance().animationPlaying = false;
    }
    if (event.getChar() == 's')
    {
        boost::filesystem::path path;
        if (event.isAltDown())
        {
            path = getSaveFilePath("", extensions);
            if (path.empty())
                return;

            if (path.extension().empty())
            {
                path += ".graph";
            }
            gh.saveGraph(path.string());
            gh.saveGraphPositions(path.replace_extension("pos").string());
        }
        else
        {
            gh.saveGraph(nameBase + ".txt");
            gh.saveGraphPositions(nameBase + ".pos");
        }
    }
    if (event.getChar() == 'l')
    {
        boost::filesystem::path path;
        if (event.isAltDown())
        {
            path = getOpenFilePath(nameBase + ".txt", extensions);
            if (path.empty())
                return;

            gh.loadGraph(path.string());
            gh.loadGraphPositions(path.replace_extension("pos").string());
        }
        else
        {
            gh.loadGraph(nameBase + ".txt");
            gh.loadGraphPositions(nameBase + ".pos");
        }

        Options::instance().startNode = 1;
        gh.fitToWindow();
    }
    if (event.getChar() == 'u')
    {
        gh.toggleAutomaticEdgeWeightUpdate();
        std::cout << "automaticEdgeWeightUpdate = " << gh.getAutomaticEdgeWeightUpdate() << std::endl;
    }
    if (event.getChar() == 'f')
    {
        gh.fitToWindow();
    }
    if (event.getChar() == 'r')
    {
        if (event.isControlDown())
        {
            if (recording)
            {
                stopRecording();
                return;
            }

            prepareRecording();
        }
        else
        {
            gh.reorderNodesSquare();
        }
    }

    if (event.getChar() == 'q')
    {
        saveSettings();
        quit();
    }
    
}



void GraphStudioApp::mouseDown( MouseEvent event )
{
    //std::cout << "GraphStudioApp::mouseDown" << std::endl;
}


void GraphStudioApp::update()
{
    gh.update();
    if (prevColorSchemeIndex != Options::instance().currentColorSchemeIdx)
    {
        prevColorSchemeIndex = Options::instance().currentColorSchemeIdx;
        Options::instance().currentColorScheme = colorSchemes[colorSchemeNames[Options::instance().currentColorSchemeIdx]];
    }
    else
    {
        storeColorScheme();
    }
}


void GraphStudioApp::draw()
{
    if (recording)
    {        
        static int imageIdx = 0;
        imageIdx++;
        std::stringstream ss;
        ss << "anim" << std::setw(4) << std::setfill('0') << imageIdx << ".png";
        ci::gl::clear(Options::instance().currentColorScheme.backgroundColor);
        std::cout << "GraphStudioApp::doRecording() " << ss.str() << std::endl;
        gh.draw();

        auto surface = copyWindowSurface();
        writeImage(ss.str(), surface);
        if (!gh.animationNext())
            stopRecording();
        return;        
    }
    // clear out the window with black
    gl::clear(Color(0, 0, 0));
    gh.draw();
    if (!Options::instance().animationPlaying || Options::instance().animationPaused)
        params->draw();
}

void GraphStudioApp::resize()
{
    gh.resize(getWindowBounds());
}

CINDER_APP_NATIVE( GraphStudioApp, RendererGl )
