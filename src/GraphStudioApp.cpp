#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "GraphHandler.h"

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
}

CINDER_APP_NATIVE( GraphStudioApp, RendererGl )
