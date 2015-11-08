#include "stdafx.h"

#include "GraphHandler.h"
#include "Options.h"
#include "generators.h"

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
	void setGraphChanged();
	void algorithmChanged();
	void algorithmStartNodeChanged();
	void colorSchemeChanged();

	void addNewColorScheme();
	void storeColorScheme();
	void createThumbnail(const fs::path &folder);
	fs::path createTempDir();
	void prepareRecording();
	void stopRecording();
	void loadSettings();
	void saveSettings();
	void saveGraph(bool saveAs);
	void loadGraph();

    static const std::vector<std::string> extensions;

    ci::params::InterfaceGlRef	params;
    std::map<std::string, ColorScheme> colorSchemes;
    std::vector<std::string> colorSchemeNames;
    GraphHandler gh;

	int minRandomEdgeWeight = 1;
	int maxRandomEdgeWeight = 100;
	int editedColorSchemeIdx = 0;
	bool showEdgeWeights = true;
	bool showNodeWeights = true;
	bool randomMovement = false;
	bool autoFitToScreen = true;
	int algorithm = static_cast<int>(Algorithm::kruskal);
	int edgeWeightScale = 100;
	bool autoEdgeWightUpdate = false;
	int framesPerState = 60;
	ColorScheme editedColorScheme;
	GraphDrawingSettings graphSettings;
	GraphDrawingSettings legendSettings;
	GraphGeneratorGrid graphGeneratorGrid;
	GraphGeneratorTriangleMesh graphGeneratorTriangleMesh;
	AnimationPlayingState playingState = AnimationPlayingState::stop;

    fs::path graphFileName;
    fs::path defaultPath;
    fs::path exportPath;
    fs::path ffmpegPath;
    fs::path configFilePath;
    fs::path videoTempDir;

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
    return colorSchemes[colorSchemeNames[editedColorSchemeIdx]];
}


void GraphStudioApp::addNewColorScheme()
{
    auto &cs = editedColorScheme;
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
    editedColorSchemeIdx = int(colorSchemes.size() - 1);
    params->addParam("ColorScheme", colorSchemeNames, &editedColorSchemeIdx);
}


void GraphStudioApp::storeColorScheme()
{
    auto &cs = editedColorScheme;
    colorSchemes[cs.name] = cs;
}


void GraphStudioApp::saveSettings()
{
    ci::XmlTree configXml("graphStudioSettings", "");
    configXml.push_back(ci::XmlTree("defaultSavePath", defaultPath.string()));
    configXml.push_back(ci::XmlTree("exportPath", exportPath.string()));
    configXml.push_back(ci::XmlTree("ffmpegPath", ffmpegPath.string()));
    configXml.push_back(ci::XmlTree("nodeSize", std::to_string(graphSettings.nodeSize)));
    configXml.push_back(ci::XmlTree("edgeWidth", std::to_string(graphSettings.edgeWidth)));
    configXml.push_back(ci::XmlTree("highlightedEdgeWidth", std::to_string(graphSettings.highlightedEdgeWidth)));
    configXml.push_back(ci::XmlTree("arrowLength", std::to_string(graphSettings.arrowLength)));
    configXml.push_back(ci::XmlTree("arrowAngle", std::to_string(graphSettings.arrowAngle)));
    configXml.push_back(ci::XmlTree("showEdgeWeights", std::to_string(showEdgeWeights)));
    configXml.push_back(ci::XmlTree("showNodeWeights", std::to_string(showNodeWeights)));
    configXml.push_back(ci::XmlTree("framesPerState", std::to_string(framesPerState)));
    configXml.push_back(ci::XmlTree("edgeWeightScale", std::to_string(edgeWeightScale)));
    configXml.push_back(ci::XmlTree("autoFitToScreen", std::to_string(autoFitToScreen)));
    
    configXml.push_back(ci::XmlTree("legendEdgeWidth", std::to_string(legendSettings.edgeWidth)));
    configXml.push_back(ci::XmlTree("legendHighlightedEdgeWidth", std::to_string(legendSettings.highlightedEdgeWidth)));
    configXml.push_back(ci::XmlTree("legendArrowLength", std::to_string(legendSettings.arrowLength)));
    configXml.push_back(ci::XmlTree("legendArrowAngle", std::to_string(legendSettings.arrowAngle)));
    configXml.push_back(ci::XmlTree("legendNodeSize", std::to_string(legendSettings.nodeSize)));

    configXml.push_back(ci::XmlTree("infoPanelWidth", std::to_string(Options::instance().infoPanelWidth)));

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
    fs::path configFile = fs::current_path() / "config.xml";
    configFilePath = configFile.generic_string();
    defaultPath = ".";// fs::current_path();
    exportPath = ".";//fs::current_path();
    ffmpegPath = "";

    if (!fs::exists(configFile))
    {
        ci::app::console() << "Cannot find config file [" << configFilePath << "]" << std::endl;
        ci::app::console() << "Loading defaults" << std::endl;

        ColorScheme cs;
        colorSchemes[cs.name] = cs;
        colorSchemeNames.push_back(cs.name);

        return;
    }
    configFilePath = configFile.generic_string();

    ci::XmlTree configXml(ci::loadFile(configFilePath));
    ci::XmlTree settings = configXml.getChild("graphStudioSettings");
    try
    {
        defaultPath = fs::path(settings.getChild("defaultSavePath").getValue()).native();
        exportPath = fs::path(settings.getChild("exportPath").getValue()).native();
        ffmpegPath = fs::path(settings.getChild("ffmpegPath").getValue()).native();
        graphSettings.nodeSize = settings.getChild("nodeSize").getValue<float>();
        graphSettings.edgeWidth = settings.getChild("edgeWidth").getValue<float>();
        graphSettings.highlightedEdgeWidth = settings.getChild("highlightedEdgeWidth").getValue<float>();
        graphSettings.arrowLength = settings.getChild("arrowLength").getValue<float>();
        graphSettings.arrowAngle = settings.getChild("arrowAngle").getValue<float>();
        showEdgeWeights = settings.getChild("showEdgeWeights").getValue<bool>();
        showNodeWeights = settings.getChild("showNodeWeights").getValue<bool>();
        autoFitToScreen = settings.getChild("autoFitToScreen").getValue<bool>();

        legendSettings.edgeWidth = settings.getChild("legendEdgeWidth").getValue<float>();
        legendSettings.highlightedEdgeWidth = settings.getChild("legendHighlightedEdgeWidth").getValue<float>();
        legendSettings.arrowLength = settings.getChild("legendArrowLength").getValue<float>();
        legendSettings.arrowAngle = settings.getChild("legendArrowAngle").getValue<float>();
        legendSettings.nodeSize = settings.getChild("legendNodeSize").getValue<float>();

        Options::instance().infoPanelWidth = settings.getChild("infoPanelWidth").getValue<float>();

		framesPerState = settings.getChild("framesPerState").getValue<int>();
        edgeWeightScale = settings.getChild("edgeWeightScale").getValue<int>();

        ci::XmlTree csList = settings.getChild("colorSchemes");
        for (auto csIt = csList.begin(); csIt != csList.end(); ++csIt)
        {
            ColorScheme cs = ColorScheme::fromXml(*csIt);
            colorSchemes[cs.name] = cs;
            colorSchemeNames.push_back(cs.name);
        }
    }
    catch (ci::XmlTree::ExcChildNotFound &e)
    {
        ci::app::console() << "Error while reading config.xml: " << e.what() << std::endl;
        ColorScheme cs;
        colorSchemes[cs.name] = cs;
        colorSchemeNames.push_back(cs.name);
    }
}


void GraphStudioApp::setGraphChanged()
{
	auto &drawer = gh.getAnimationDrawer();
	drawer.setColorScheme(editedColorScheme);
	drawer.setDrawingSettings(graphSettings);
	drawer.setShowEdgeWeights(showEdgeWeights);
	drawer.setShowNodeWeights(showNodeWeights);
	GraphNodeHandler::setSize(graphSettings.nodeSize);
}

void GraphStudioApp::algorithmChanged()
{
    gh.algorithmChanged(Algorithm(algorithm));
	colorSchemeChanged();
}

void GraphStudioApp::algorithmStartNodeChanged()
{
    gh.algorithmStartNodeChanged();
}

void GraphStudioApp::colorSchemeChanged()
{
    editedColorScheme = colorSchemes[colorSchemeNames[editedColorSchemeIdx]];
    gh.getAnimationDrawer().setColorScheme(editedColorScheme);
}

void GraphStudioApp::setup()
{
    loadSettings();
    std::function<void()> updaterFunction = std::bind(&GraphStudioApp::setGraphChanged, this);

    params = params::InterfaceGl::create("Graph Studio", ci::ivec2(200, 310));
    params->addParam<float>("Node Size", &graphSettings.nodeSize).updateFn(updaterFunction).min(1.0f).max(50.0f).step(1.0f);
    params->addParam<float>("Edge Width", &graphSettings.edgeWidth).updateFn(updaterFunction).min(0.1f).max(10.0f).step(0.1f);
    params->addParam<float>("Highlighted Edge Width", &graphSettings.highlightedEdgeWidth).updateFn(updaterFunction).min(0.0f).max(10.0f).step(0.1f);
    params->addParam<float>("Arrow Length", &graphSettings.arrowLength).updateFn(updaterFunction).min(1.0f).max(50.0f).step(1.0f);
    params->addParam<float>("Arrow Angle", &graphSettings.arrowAngle).updateFn(updaterFunction).min(0.0f).max(90.0f).step(1.0f);
    params->addParam<bool>("Show Edge Weights", &showEdgeWeights).updateFn(updaterFunction);
    params->addParam<bool>("Show Node Weights", &showNodeWeights).updateFn(updaterFunction);

    params->addSeparator();
    params->addParam("Algorithm", AlgorithmNames, &algorithm).updateFn(std::bind(&GraphStudioApp::algorithmChanged, this));
    params->addParam<int>("Starting Node", &Options::instance().startNode).min(1).step(1).updateFn(std::bind(&GraphStudioApp::algorithmStartNodeChanged, this));
    params->addSeparator();
	params->addParam<int>("Frames Per State", &framesPerState).updateFn([&]() {gh.getAnimationDrawer().setFramesPerState(framesPerState); }).min(1).max(300).step(1);
    params->addParam<int>("Edge Weight Scale", &edgeWeightScale).updateFn(updaterFunction).min(1.0f).max(1000.0f).step(1.0f);
    params->addSeparator();
    params->addText("Colors");
    params->addParam("ColorScheme", colorSchemeNames, &editedColorSchemeIdx).updateFn(std::bind(&GraphStudioApp::colorSchemeChanged, this));
    params->addParam<ci::Color>("Background", &editedColorScheme.backgroundColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Legend Background", &editedColorScheme.legendBackgroundColor).updateFn(updaterFunction);

    params->addParam<ci::Color>("Node ", &editedColorScheme.nodeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 1", &editedColorScheme.highlightedNodeColor1).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 2", &editedColorScheme.highlightedNodeColor2).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Node 3", &editedColorScheme.highlightedNodeColor3).updateFn(updaterFunction);

    params->addParam<ci::Color>("Edge", &editedColorScheme.edgeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Dark Edge", &editedColorScheme.darkEdgeColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 1", &editedColorScheme.highlightedEdgeColor1).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 2", &editedColorScheme.highlightedEdgeColor2).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge 3", &editedColorScheme.highlightedEdgeColor3).updateFn(updaterFunction);

    params->addParam<ci::Color>("Node Text", &editedColorScheme.nodeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("highlightednodeText", &editedColorScheme.highlightednodeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Edge Text", &editedColorScheme.edgeTextColor).updateFn(updaterFunction);
    params->addParam<ci::Color>("Highlighted Edge Text", &editedColorScheme.highlightedEdgeTextColor).updateFn(updaterFunction);

    params->addButton("New", std::bind(&GraphStudioApp::addNewColorScheme, this));
    params->addSeparator();
    params->addText("Random Edge Weights");
    params->addParam<int>("Min", &minRandomEdgeWeight).updateFn(updaterFunction).min(1).max(1000).step(1);
    params->addParam<int>("Max", &maxRandomEdgeWeight).updateFn(updaterFunction).min(1).max(1000).step(1);
	params->addButton("Generate Weights", [&](){ gh.setRandomEdgeWeights(minRandomEdgeWeight, maxRandomEdgeWeight); });
    params->addSeparator();
    params->addText("Generate Grid");
    params->addParam<int>("Columns", &graphGeneratorGrid.columns).min(1).step(1);
    params->addParam<int>("Rows", &graphGeneratorGrid.rows).min(1).step(1);
    params->addParam<bool>("Directed", &graphGeneratorGrid.directed);
    params->addParam<bool>("Horizontal Edges", &graphGeneratorGrid.horizontal);
    params->addParam<bool>("Vertical Edges", &graphGeneratorGrid.vertical);
    params->addParam<bool>("Diagonal /", &graphGeneratorGrid.upDiagonal);
    params->addParam<bool>("Diagonal \\", &graphGeneratorGrid.downDiagonal);
    params->addButton("Generate grid", [&](){gh.generateSpecialGraph(graphGeneratorGrid); });
    params->addText("Generate Triangle Mesh");
    params->addParam<int>("Triangles", &graphGeneratorTriangleMesh.triangles).min(1).step(1);
    params->addParam<bool>("Directed", &graphGeneratorTriangleMesh.directed);
    params->addParam<float>("Randomness", &graphGeneratorTriangleMesh.randomness).min(0.0f).step(0.1f);
    params->addButton("Generate tri", [&](){gh.generateSpecialGraph(graphGeneratorTriangleMesh); });

    gh.setup(getWindow());
	gh.getAnimationDrawer().setFramesPerState(framesPerState);
    colorSchemeChanged();
    algorithmChanged();
	setGraphChanged();
}

GraphStudioApp::~GraphStudioApp()
{
    saveSettings();
}

void GraphStudioApp::stopRecording()
{
	playingState = AnimationPlayingState::stop;
    gh.animationPause();
    setFullScreen(false);
    //showCursor();
    params->show();
    setFrameRate(30.0f);
}


fs::path GraphStudioApp::createTempDir()
{
	fs::path ret = fs::temp_directory_path();
	int counter = 0;
	while (fs::exists(ret))
	{
		std::string candidate = "GrahStudio_" + std::to_string(++counter);
		ret = fs::temp_directory_path() / candidate;
	}
	return ret;
}

void GraphStudioApp::prepareRecording()
{
    setFrameRate(30.0f);
	playingState = AnimationPlayingState::recording;
    setFullScreen(true);
    params->hide();

	videoTempDir = createTempDir();
	playingState = AnimationPlayingState::play;
    gh.animationPrepare();
    gh.animationGoToFirst();
    gh.animationPause();
}

void GraphStudioApp::createThumbnail(const fs::path &folder)
{
    bool origFullscreenState = isFullScreen();
    bool origParamsVisible = params->isVisible();

	playingState = AnimationPlayingState::stop;
    gh.animationPrepare();
    gh.animationGoToLast();
    gh.animationPause();

    params->hide();
    setFullScreen(false);
    auto origWindowSize = getWindowSize();
    setWindowSize(800, 450);

    //ci::gl::clear(editedColorScheme.backgroundColor);
    auto &graphDrawer = gh.getAnimationDrawer();
    bool origAnimationStateNumberVisible = graphDrawer.getShowAnimationStateNumber();
    bool origAnimationStateDescriptionVisible = graphDrawer.getShowAnimationStateDescription();
    bool origLegendVisible = graphDrawer.getShowLegend();
    graphDrawer.showAnimationStateNumber(false);
    graphDrawer.showAnimationStateDescription(false);
    graphDrawer.showLegend(false);

	gh.getAnimationDrawer().setShowEdgeWeights(false);
	gh.getAnimationDrawer().setShowNodeWeights(false);
    gh.draw();

    auto surface = copyWindowSurface();
    fs::path fullPath(folder);
    fullPath /= "thumbnail.png";
    writeImage(fullPath.c_str(), surface);

    setFullScreen(origFullscreenState);
    setWindowSize(origWindowSize);
    graphDrawer.showAnimationStateNumber(origAnimationStateNumberVisible);
    graphDrawer.showAnimationStateDescription(origAnimationStateDescriptionVisible);
    graphDrawer.showLegend(origLegendVisible);

	gh.getAnimationDrawer().setShowEdgeWeights(showEdgeWeights);
	gh.getAnimationDrawer().setShowNodeWeights(showNodeWeights);
    if (origParamsVisible)
        params->show();
}


void GraphStudioApp::saveGraph(bool saveAs)
{
    fs::path path;
    if (graphFileName.empty() || saveAs)
    {
        defaultPath = defaultPath.native();
        if (!fs::exists(defaultPath))
        {
            fs::create_directory(defaultPath);
        }
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
        fs::path fullPath = defaultPath;
        fullPath /= graphFileName;
        gh.saveGraph(graphFileName);
        gh.saveGraphPositions(fullPath.replace_extension("pos").generic_string());
    }
}

void GraphStudioApp::loadGraph()
{
    fs::path path = defaultPath;
    path = getOpenFilePath(path / graphFileName, extensions);
    if (path.empty())
        return;
    
    defaultPath = path.parent_path().generic_string();
    graphFileName = path.filename().string();
    gh.loadGraph(path.string());
    gh.loadGraphPositions(path.replace_extension("pos").string());

    gh.algorithmChanged(Algorithm(algorithm));
	gh.getAnimationDrawer().setFramesPerState(framesPerState);
	colorSchemeChanged();
    if (autoFitToScreen)
    {
        gh.fitToWindow();
    }
}

void GraphStudioApp::keyDown(KeyEvent event)
{

	if (event.getCode() == KeyEvent::KEY_SPACE)
	{
		switch (playingState)
		{
		default:
			break;
		case AnimationPlayingState::stop:
			playingState = AnimationPlayingState::play;
			gh.disconnectMouseEvents();
			gh.animationGoToFirst();
			gh.animationResume();
			break;
		case AnimationPlayingState::play:
			playingState = AnimationPlayingState::pause;
			gh.connectMouseEvents();
			gh.animationPause();
			break;
		case AnimationPlayingState::pause:
			playingState = AnimationPlayingState::play;
			gh.disconnectMouseEvents();
            gh.animationResume();            
			break;
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
		playingState = AnimationPlayingState::stop;
        gh.animationGoToLast();
		gh.connectMouseEvents();
        return;
    }
    if (event.getChar() == 'm')
    {
        randomMovement = !randomMovement;
		if (randomMovement)
		{
			playingState = AnimationPlayingState::stop;
			gh.startRandomMovement();
		}
		else
		{
			gh.stopRandomMovement();
		}
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
		autoEdgeWightUpdate = !autoEdgeWightUpdate;
    }
    if (event.getChar() == 'f')
    {
        gh.fitToWindow();
    }
    if (event.getChar() == 't')
    {
        createThumbnail(fs::current_path().string());
    }
    if (event.getChar() == 'r')
    {
        if (event.isControlDown())
        {
            if (playingState == AnimationPlayingState::recording)
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


void GraphStudioApp::mouseDown(MouseEvent event)
{
    //ci::app::console() << "GraphStudioApp::mouseDown" << std::endl;
}


void GraphStudioApp::update()
{
	if (autoEdgeWightUpdate)
		gh.setEdgeWeightsFromLengths(edgeWeightScale / 100.0);
    gh.update();
    storeColorScheme();
}


void GraphStudioApp::draw()
{
    if (playingState == AnimationPlayingState::recording)
    {
        std::stringstream ss;
        ss << "anim" << std::setw(4) << std::setfill('0') << gh.getAnimationDrawer().getAnimationStateNumber() + 1 << ".png";
        ci::gl::clear(editedColorScheme.backgroundColor);

        gh.draw();

        auto surface = copyWindowSurface();
        fs::path fullPath = videoTempDir;
        fullPath /= ss.str();
        writeImage(fullPath.string(), surface);
        ci::app::console() << "GraphStudioApp::doRecording() " << fullPath.string() << std::endl;
        if (!gh.animationNext())
        {
            stopRecording();
            std::stringstream command;
            fs::path animFileBase = videoTempDir;
            animFileBase /= "anim";
            fs::path videoFullPath = videoTempDir;
            std::string graphFileNameNoExtension = fs::path(graphFileName).stem().string();

            if (!fs::exists(ffmpegPath) || !fs::is_regular_file(ffmpegPath))
            {
                console() << "Cannot find ffmpeg.exe executable [ffmpegPath=" << ffmpegPath << "]" << std::endl;
                return;
            }

            command << ffmpegPath.string() << " -framerate 1/2 -i \"" << animFileBase.string() << "%04d.png\" -c:v libx264" <<
                " -crf 18 -pix_fmt yuv420p -r 30 " << (videoFullPath / (graphFileNameNoExtension + ".mp4")).string();

            console() << "Executing:" << std::endl << command.str() << std::endl;
            system(command.str().c_str());
            fs::path fsExportPath = exportPath;
            createThumbnail(videoTempDir);
            if (fs::exists(fsExportPath) && fs::is_regular_file(fsExportPath))
            {
                std::cerr << "Cannot write to export path [" << fsExportPath.string() << "]" << std::endl;
                return;
            }
            if (!fs::exists(fsExportPath))
            {
                fs::create_directory(fsExportPath);
            }
			using copy_option = std::experimental::filesystem::v1::copy_options;
            fs::copy_file(videoFullPath / (graphFileNameNoExtension + ".mp4"), fsExportPath / (graphFileNameNoExtension + ".mp4"), copy_option::overwrite_existing);
            fs::copy_file(fs::path(videoTempDir) / "thumbnail.png", fsExportPath / (graphFileNameNoExtension + ".png"), copy_option::overwrite_existing);
            fs::copy_file(fs::path(defaultPath) / (graphFileNameNoExtension + ".graph"), fsExportPath / (graphFileNameNoExtension + ".graph"), copy_option::overwrite_existing);
            fs::copy_file(fs::path(defaultPath) / (graphFileNameNoExtension + ".pos"), fsExportPath / (graphFileNameNoExtension + ".pos"), copy_option::overwrite_existing);

            fs::remove_all(fs::path(videoTempDir));
        }
    }
    else
    {
        // clear out the window with black
        gl::clear(Color(0, 0, 0));
        gh.draw();
        if (playingState != AnimationPlayingState::play)
            params->draw();
		if (gh.getAnimationDrawer().isAnimationFinished())
		{
			playingState = AnimationPlayingState::stop;
		}
    }
}

void GraphStudioApp::resize()
{
    gh.resize(getWindowBounds());
}

CINDER_APP(GraphStudioApp, RendererGl(RendererGl::Options().msaa(8)))
