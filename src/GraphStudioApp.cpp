#include "cinder/app/AppNative.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "GraphHandler.h"
#include "Options.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GraphStudioApp : public AppNative {
public:
    void prepareSettings(Settings *settings);
    void setup();
    void mouseDown(MouseEvent event);
    void keyDown(KeyEvent event);
    void update();
    void draw();
private:
    ci::params::InterfaceGlRef	params;

    GraphHandler gh;
};


void GraphStudioApp::prepareSettings(Settings *settings)
{
    settings->enableConsoleWindow();
    settings->setWindowSize(800, 600);
    settings->setFrameRate(120.0f);
}


void GraphStudioApp::setup()
{
    params = params::InterfaceGl::create("Graph Studio", Vec2i(200, 310));
    params->addParam("Node Size", &Options::instance().nodeSize, "min=1.0 max=50.0 step=1.0 keyIncr=n keyDecr=N");
    params->addParam("Arrow Size", &Options::instance().arrowSize, "min=1.0 max=50.0 step=1.0 keyIncr=n keyDecr=N");
    params->addSeparator();
    params->addParam("Speed", &Options::instance().speed, "min=100.0 max=2000.0 step=50.0 keyIncr=s keyDecr=w");
    params->addSeparator();
    params->addParam("Node ", &Options::instance().nodeColor);
    params->addParam("Highlighted Node ", &Options::instance().highlightedNodeColor);
    params->addParam("Edge ", &Options::instance().edgeColor);
    params->addParam("Highlighted Edge ", &Options::instance().highlightedEdgeColor);
    gh.prepare(getWindow());

}

void GraphStudioApp::keyDown(KeyEvent event)
{
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
        gh.loadGraph("graphPE81.txt");
        gh.loadGraphPositions("graphPE81.pos");
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
}

CINDER_APP_NATIVE( GraphStudioApp, RendererGl )
