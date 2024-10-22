#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){

	ofGLFWWindowSettings settings;
	settings.setSize(1920, 1080);
	settings.setPosition(glm::vec2(300,0));
	settings.resizable = true;
	auto mainWindow = ofCreateWindow(settings);

	settings.setSize(800, 800);
	settings.setPosition(glm::vec2(0,0));
	settings.resizable = false;
	// uncomment next line to share main's OpenGL resources with gui
	//settings.shareContextWith = mainWindow;
	auto guiWindow = ofCreateWindow(settings);
	guiWindow->setVerticalSync(false);

	auto mainApp = make_shared<ofApp>();
	mainApp->setupGui();
	ofAddListener(guiWindow->events().draw,mainApp.get(),&ofApp::drawGui);

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();

}