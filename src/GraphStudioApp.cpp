#include "cinder/app/AppNative.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/qtime/MovieWriter.h"
#include "GraphHandler.h"
#include "Options.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GraphStudioApp : public AppNative {
public:
    void prepareSettings(Settings *settings) override;
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void update() override;
    void draw() override;
    void resize() override;
private:
    ci::params::InterfaceGlRef	params;
    qtime::MovieWriterRef	mMovieWriter;
    
    GraphHandler gh;
};


void GraphStudioApp::prepareSettings(Settings *settings)
{
    settings->enableConsoleWindow();
    settings->setWindowSize(800, 600);
    settings->setFrameRate(60.0f);
}


void GraphStudioApp::setup()
{
    params = params::InterfaceGl::create("Graph Studio", Vec2i(200, 310));
    params->addParam("Node Size", &Options::instance().nodeSize, "min=1.0 max=50.0 step=1.0");
    params->addParam("Edge Width", &Options::instance().edgeWidth, "min=0.0 max=10.0 step=0.1");
    params->addParam("Highlighted Edge Width", &Options::instance().highlighedEdgeWidth, "min=0.0 max=10.0 step=0.1");
    params->addParam("Arrow Length", &Options::instance().arrowLength, "min=1.0 max=50.0 step=1.0");
    params->addParam("Arrow Angle", &Options::instance().arrowAngle, "min=0.0 max=90.0 step=1.0");

    
    params->addSeparator();
    params->addParam("Force", &Options::instance().force, "min=1.0 max=300.0 step=1.0");
    params->addParam("Speed", &Options::instance().speed, "min=1.0 max=300.0 step=1.0");
    params->addSeparator();
    params->addParam("Background", &Options::instance().backgroundColor);
    params->addParam("Node ", &Options::instance().nodeColor);
    params->addParam("Highlighted Node ", &Options::instance().highlightedNodeColor);
    params->addParam("Edge ", &Options::instance().edgeColor);
    params->addParam("Highlighted Edge ", &Options::instance().highlightedEdgeColor);
    params->addSeparator();
    params->addText("Generate Grid");
    params->addParam("Columns", &GridGraphParams::instance().columns, "min=1 step=1");
    params->addParam("Rows", &GridGraphParams::instance().rows, "min=1 step=1");
    params->addParam("Directed", &GridGraphParams::instance().directed);
    params->addParam("Horizontal Edges", &GridGraphParams::instance().horizontal);
    params->addParam("Vertical Edges", &GridGraphParams::instance().vertical);
    params->addParam("Diagonal /", &GridGraphParams::instance().upDiagonal);
    params->addParam("Diagonal \\", &GridGraphParams::instance().downDiagonal);
    params->addButton("Generate", std::bind(&GraphHandler::generateGrid, &gh));

    gh.prepare(getWindow());
    /*
    fs::path path = getSaveFilePath();
    if (path.empty())
        return; // user cancelled save
    qtime::MovieWriter::Format format;
    if (qtime::MovieWriter::getUserCompressionSettings(&format)) {
        mMovieWriter = qtime::MovieWriter::create(path, getWindowWidth(), getWindowHeight(), format);
    }
    */
}

void GraphStudioApp::keyDown(KeyEvent event)
{
    if (event.getCode() == KeyEvent::KEY_SPACE)
    {
        Options::instance().animationPlaying = !Options::instance().animationPlaying;
        gh.prepareAnimation();
        return;
    }
    if (event.getChar() == 's')
    {
        std::cout << "Saving graph..." << std::endl;
        gh.saveGraph("graph.txt");
        gh.saveGraphPositions("graph.pos");
        std::cout << "Done" << std::endl;
    }
    if (event.getChar() == 'l')
    {
        std::cout << "Loading graph..." << std::endl;
        gh.loadGraph("graph.txt");
        gh.loadGraphPositions("graph.pos");
        std::cout << "Done" << std::endl;
    }
    if (event.getChar() == 'u')
    {
        gh.toggleAutomaticEdgeWeightUpdate();
        std::cout << "automaticEdgeWeightUpdate = " << gh.getAutomaticEdgeWeightUpdate() << std::endl;
    }
    if (event.getChar() == 'r')
    {
        gh.reorderNodesSquare();
    }
    if (event.getChar() == 'q')
    {
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
}


void GraphStudioApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    gh.draw();
    params->draw();
    /*
    if (mMovieWriter)
        mMovieWriter->addFrame(copyWindowSurface());
    */
}

void GraphStudioApp::resize()
{
    gh.resize(getWindowBounds());
}

CINDER_APP_NATIVE( GraphStudioApp, RendererGl )
