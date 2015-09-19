#include "stdafx.h"

#include "GraphHandler.h"
#include "Options.h"

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/params/Params.h>
#include <cinder/ImageIo.h>

using namespace ci;
using namespace ci::app;

class GraphStudioApp : public App {
public:
    ~GraphStudioApp();

    static void prepareSettings(Settings *settings);
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;
    void resize() override;
    
    const ColorScheme &getCurrentColorScheme();
private:
    ci::params::InterfaceGlRef	params;
    std::map<std::string, ColorScheme> colorSchemes;
    std::vector<std::string> colorSchemeNames;
    GraphHandler gh;

    static const std::vector<std::string> extensions;
    std::string graphFileName;
    std::string defaultPath;
    std::string configFilePath;

    void setGraphChanged();
    void algorithmChanged();
    void algorithmStartNodeChanged();
    void colorSchemeChanged();

    bool recording = false;
    void addNewColorScheme();
    void storeColorScheme();
    void prepareRecording();
    void stopRecording();
    void loadSettings();
    void saveSettings();
    void saveGraph(bool saveAs);
    void loadGraph();

};

const std::vector<std::string> GraphStudioApp::extensions{ "graph", "txt" };


void GraphStudioApp::prepareSettings(Settings *settings)
{
    settings->setConsoleWindowEnabled(true);
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
    configXml.push_back(ci::XmlTree("defaultSavePath", defaultPath));
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

    configXml.write(ci::writeFile(configFilePath));
}


void GraphStudioApp::loadSettings()
{    
    boost::filesystem::path configFile = boost::filesystem::current_path() / "config.xml";    
    configFilePath = configFile.generic_string();
    if (!boost::filesystem::exists(configFile))
    { 
        std::cout << "Cannot find config file [" << configFilePath << "]" << std::endl;
        return;
    }
    configFilePath = configFile.generic_string();

    ci::XmlTree configXml(ci::loadFile(configFilePath));
    ci::XmlTree settings = configXml.getChild("graphStudioSettings");
    defaultPath = settings.getChild("defaultSavePath").getValue();
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


void GraphStudioApp::setGraphChanged()
{
    gh.setChanged();
}

void GraphStudioApp::algorithmChanged()
{
    gh.algorithmChanged();
}

void GraphStudioApp::algorithmStartNodeChanged()
{
    gh.algorithmStartNodeChanged();
}

void GraphStudioApp::colorSchemeChanged()
{
    Options::instance().currentColorScheme = colorSchemes[colorSchemeNames[Options::instance().currentColorSchemeIdx]];
}

void GraphStudioApp::setup()
{    
    loadSettings();
    std::function<void()> updaterFunction = std::bind(&GraphStudioApp::setGraphChanged, this);

    params = params::InterfaceGl::create("Graph Studio", ci::ivec2(200, 310));
    params->addParam<float>("Node Size", &Options::instance().nodeSize).updateFn(updaterFunction).min(1.0f).max(50.0f).step(1.0f);
    params->addParam<float>("Edge Width", &Options::instance().edgeWidth).updateFn(updaterFunction).min(0.1f).max(10.0f).step(0.1f);
    params->addParam<float>("Highlighted Edge Width", &Options::instance().highlighedEdgeWidth).updateFn(updaterFunction).min(0.0f).max(10.0f).step(0.1f);
    params->addParam<float>("Arrow Length", &Options::instance().arrowLength).updateFn(updaterFunction).min(1.0f).max(50.0f).step(1.0f);
    params->addParam<float>("Arrow Angle", &Options::instance().arrowAngle).updateFn(updaterFunction).min(0.0f).max(90.0f).step(1.0f);
    params->addParam<bool>("Show Edge Weights", &Options::instance().showEdgeWeights).updateFn(updaterFunction);
    params->addParam<bool>("Show Node Weights", &Options::instance().showNodeWeights).updateFn(updaterFunction);

    params->addSeparator();
    params->addParam("Algorithm", AlgorithmNames, &Options::instance().algorithm).updateFn(std::bind(&GraphStudioApp::algorithmChanged, this));
    params->addParam<int>("Starting Node", &Options::instance().startNode).min(1).step(1).updateFn(std::bind(&GraphStudioApp::algorithmStartNodeChanged, this));
    params->addSeparator();
    params->addParam<float>("Force", &Options::instance().force).updateFn(updaterFunction).min(1.0f).max(300.0f).step(1.0f);
    params->addParam<int>("Speed", &Options::instance().speed).updateFn(updaterFunction).min(1).max(300).step(1);
    params->addParam<int>("Edge Weight Scale", &Options::instance().edgeWeightScale).updateFn(updaterFunction).min(1.0f).max(1000.0f).step(1.0f);
    params->addSeparator();
    params->addText("Colors");
    params->addParam("ColorScheme", colorSchemeNames, &Options::instance().currentColorSchemeIdx).updateFn(std::bind(&GraphStudioApp::colorSchemeChanged, this));
    params->addParam<ci::Color>("Background", &Options::instance().currentColorScheme.backgroundColor);

    params->addParam<ci::Color>("Node ", &Options::instance().currentColorScheme.nodeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 1", &Options::instance().currentColorScheme.highlightedNodeColor1).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 2", &Options::instance().currentColorScheme.highlightedNodeColor2).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 3", &Options::instance().currentColorScheme.highlightedNodeColor3).updateFn(updaterFunction);

    params->addParam<ci::Color>("Edge", &Options::instance().currentColorScheme.edgeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Dark Edge", &Options::instance().currentColorScheme.darkEdgeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 1", &Options::instance().currentColorScheme.highlightedEdgeColor1).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 2", &Options::instance().currentColorScheme.highlightedEdgeColor2).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 3", &Options::instance().currentColorScheme.highlightedEdgeColor3).updateFn(updaterFunction);

    params->addParam<ci::Color>("Node Text", &Options::instance().currentColorScheme.nodeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("highlightednodeText", &Options::instance().currentColorScheme.highlightednodeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Edge Text", &Options::instance().currentColorScheme.edgeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge Text", &Options::instance().currentColorScheme.highlightedEdgeTextColor).updateFn(updaterFunction);

    params->addButton("New", std::bind(&GraphStudioApp::addNewColorScheme, this));
    params->addSeparator();
    params->addText("Random Edge Weights");
    params->addParam<float>("Min", &Options::instance().minRandomEdgeWeight).updateFn(updaterFunction).min(1.0f).max(1000.0f).step(1.0f);
    params->addParam<float>("Max", &Options::instance().maxRandomEdgeWeight).updateFn(updaterFunction).min(1.0f).max(1000.0f).step(1.0f);
    params->addButton("Generate Weights", std::bind(&GraphHandler::setRandomEdgeWeights, &gh));
    params->addSeparator();
    params->addText("Generate Grid");
    params->addParam<int>("Columns", &GraphParamsGrid::instance().columns).min(1).step(1);
    params->addParam<int>("Rows", &GraphParamsGrid::instance().rows).min(1).step(1);
    params->addParam<bool>("Directed", &GraphParamsGrid::instance().directed);
    params->addParam<bool>("Horizontal Edges", &GraphParamsGrid::instance().horizontal);
    params->addParam<bool>("Vertical Edges", &GraphParamsGrid::instance().vertical);
    params->addParam<bool>("Diagonal /", &GraphParamsGrid::instance().upDiagonal);
    params->addParam<bool>("Diagonal \\", &GraphParamsGrid::instance().downDiagonal);
    params->addButton("Generate grid", std::bind(&GraphHandler::generateSpecialGraph, &gh, GraphHandler::GraphType::grid));
    params->addText("Generate Triangle Mesh");
    params->addParam<int>("Triangles", &GraphParamsTriangleMesh::instance().triangles).min(1).step(1);
    params->addParam<float>("Randomness", &GraphParamsTriangleMesh::instance().randomness).min(0.0f).step(0.1f);
    params->addButton("Generate tri", std::bind(&GraphHandler::generateSpecialGraph, &gh, GraphHandler::GraphType::triangleMesh));

    gh.setup(getWindow());
    colorSchemeChanged();
    algorithmChanged();
}

GraphStudioApp::~GraphStudioApp()
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

void GraphStudioApp::saveGraph(bool saveAs)
{
    boost::filesystem::path path;
    if (graphFileName.empty() || saveAs)
    {
        path = getSaveFilePath(defaultPath, extensions);
        if (path.empty())
            return;

        if (path.extension().empty())
        {
            path += ".graph";
        }
        graphFileName = path.filename().string();
        gh.saveGraph(path.string());
        gh.saveGraphPositions(path.replace_extension("pos").string());
    }
    else
    {
        boost::filesystem::path fullPath = defaultPath;
        fullPath /= graphFileName;
        gh.saveGraph(graphFileName);
        gh.saveGraphPositions(fullPath.replace_extension("pos").generic_string());
    }
}

void GraphStudioApp::loadGraph()
{
    boost::filesystem::path path = defaultPath;
    path = getOpenFilePath(path / graphFileName, extensions);
    if (path.empty())
        return;

    defaultPath = path.parent_path().generic_string();
    graphFileName = path.filename().string();
    gh.loadGraph(path.string());
    gh.loadGraphPositions(path.replace_extension("pos").string());
        
    gh.algorithmChanged();
    gh.fitToWindow();
}

void GraphStudioApp::keyDown(KeyEvent event)
{
    
    if (event.getCode() == KeyEvent::KEY_SPACE)
    {
        if (!Options::instance().animationPlaying)
        {
            gh.animationGoToFirst();
            Options::instance().animationPlaying = true;
            Options::instance().animationPaused = false;
            gh.animationResume();
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
    else  if (event.getCode() == KeyEvent::KEY_HOME)
    {
        gh.animationGoToFirst();
        return;
    }
    else  if (event.getCode() == KeyEvent::KEY_END || event.getCode() == KeyEvent::KEY_ESCAPE)
    {
        stopRecording();
        Options::instance().animationPlaying = false;
        Options::instance().animationPaused = false;
        gh.animationGoToLast();
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
        saveGraph(event.isAltDown());
    }
    if (event.getChar() == 'l')
    {
        loadGraph();
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
    storeColorScheme();
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

CINDER_APP(GraphStudioApp, RendererGl(RendererGl::Options().msaa(8)))
