#include "ofMain.h"
#include "ofAppGLFWWindow.h"

#include "ofxGui.h"
#include "ofxMaxim.h"

class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 640;
        height = 480;
        grabber.initGrabber(width, height);
        
        depths.resize(width * height);
        ofSetVerticalSync(true);
        
        fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 8);
        fbo.begin();
        ofClear(100, 100, 100, 0);
        fbo.end();
        
        shader.load("DOFCloud");
        
        cam.setNearClip(0);
        cam.setFarClip(1000);
        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
        mesh.setUsage(GL_STREAM_DRAW);
        
        gradient.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
        gradient.setUsage(GL_STREAM_DRAW);

        light.setPosition(1000, 1000, 2000);
        
        ofSetBackgroundAuto(false);
    }
    
    void setupGUI() {
        gui.setup("panel");
        gui.add(focus_dist.set("focus-dist", 230, 0, 1000));
        gui.add(aperture.set("aperture", 0.06, 0.001, 0.2));
        gui.add(fov.set("fov", 30, 30, 180));
        gui.add(drag.set("drag", 1.0, 0.0, 1.0));
        gui.add(circle.set("line-cirle", 1.0, 0.0, 1.0));
        gui.add(depth_scale.set("depth-scale", 0.2, 0.01, 1));
        gui.add(depth_interp.set("depth-interp", 0.2, 0.01, 1));
        gui.add(point_size.set("point-size", 0.1, 0.01, 20));
        color1 = ofColor(150);
        color2 = ofColor(120);
        gui.add(color1);
        gui.add(color2);

        b_setup = true;
    }
    
    void update() {
        if(!b_setup)
            return;
    }
    
    void draw()
    {
        if(!b_setup)
            return;
        grabber.update();
        if (grabber.isFrameNew()) {
            drawPoints();
            
            cam.setFov(fov);
            cam.setDrag(drag);
            
            fbo.begin();
            ofSetColor(160, 150, 145, 200);
            ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            
            ofSetColor(255);
            glPushAttrib(GL_ENABLE_BIT);
            glEnable(GL_POINT_SMOOTH); // makes circular points
            glEnable(GL_CULL_FACE);
            glPointSize(point_size);
            cam.begin();
            light.enable();
            ofScale(1, -1, 1); // make y point down
            shader.begin();
            shader.setUniform1f("focus_dist", focus_dist);
            shader.setUniform1f("aperture", aperture);
            mesh.draw();
            shader.end();
            light.disable();
            cam.end();
            glPopAttrib();
            fbo.end();
        }
        
        fbo.draw(0, 0);
        
        ofSetWindowTitle(ofToString(ofGetFrameRate()));
    }
    
    void drawPoints() {
        auto pixels = grabber.getPixels();
        float radius = 200;
        mesh.clearColors();
        mesh.clearVertices();
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                ofColor cur = pixels.getColor(x, y);
                float previous_depth = depths[y * width + x];
                float this_depth = cur.r * 0.28 + cur.g * 0.6 + cur.b * 0.12;
                float depth = previous_depth * depth_interp + this_depth * (1 - depth_interp);
                int idx = y * width + x;
                depths[idx] = depth;
                if(depth > 20 && depth < 200) {
                        // the alpha value encodes depth, let's remap it to a good depth range
                    float z = ofMap(depth, 0, 255, -1000 * depth_scale, 1000 * depth_scale);
                    cur.a = depth;
                    mesh.addColor(cur);
                    float sx = ofMap(x, 0, width, 0, 2 * PI);
                    float sy = ofMap(y, 0, height, 0, PI);
                    float cx = (radius + depth) * cos(sx) * sin(sy) * depth_scale;
                    float cy = (radius + depth) * sin(sx) * sin(sy) * depth_scale;
                    float cz = (radius + depth) * cos(sy) * depth_scale;
                    float lx = width - x - width / 2;
                    float ly = y - height / 2;
                    float lz = z;
                    float fx = (1 - circle) * lx + circle * cx;
                    float fy = (1 - circle) * ly + circle * cy;
                    float fz = (1 - circle) * lz + circle * cz;
                    ofVec3f pos(fx, fy, fz);
                    mesh.addVertex(pos);
                }
            }
        }
    }
    
    void drawGUI(ofEventArgs & args) {
        gui.draw();
    }
    
    void keyPressed(int key) {
        if(key == 'f'){
            ofToggleFullscreen();
        }
        else if(key == 's') {
            gui.saveToFile("settings.xml");
        }
        else if(key == 'l') {
            gui.loadFromFile("settings.xml");
        }
    }
    
    void mousePressed(ofMouseEventArgs args) {
        gui.mousePressed(args);
    }
    
private:
    
    vector<float> depths;

    int width, height;
    
    ofVideoGrabber grabber;
    ofEasyCam cam;
    ofVboMesh mesh, gradient;
    ofShader shader;
    ofFbo fbo;
    
    ofParameter<float> focus_dist,
        aperture,
        x, y, z,
        fov,
        drag,
        distance,
        depth_scale,
        depth_interp,
        circle,
        point_size;
    ofParameter<int> circle_res;
    ofParameter<ofColor> color1, color2;
    ofxPanel gui;
    
    ofLight light;
    
    bool b_setup;
};


//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.width = 1200;
    settings.height = 600;
    settings.setPosition(ofVec2f(300,0));
    settings.resizable = true;
    settings.setGLVersion(2, 0);
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    settings.width = 220;
    settings.height = 440;
    settings.setPosition(ofVec2f(0,0));
    settings.resizable = false;
        // uncomment next line to share main's OpenGL resources with gui
        //settings.shareContextWith = mainWindow;
    shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
    guiWindow->setVerticalSync(false);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupGUI();
    ofAddListener(guiWindow->events().draw, mainApp.get(), &ofApp::drawGUI);
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();

}
