#include "ofMain.h"


class ofApp : public ofBaseApp {
public:
    void setup() {
        width = 640;
        height = 480;
        grabber.initGrabber(width, height);
        
        depths.resize(width * height);
        
        ofSetVerticalSync(true);
        
        shader.load("DOFCloud");
        focusDistance = 500;
        aperture = .2;
        
        glEnable(GL_DEPTH_TEST);
        glPointSize(3);
    }
    void update() {
        
        aperture = mouseX / (float)ofGetWidth() * 0.3;
        focusDistance = mouseY / (float)ofGetHeight() * 400.0;
        
        grabber.update();
        if (grabber.isFrameNew()) {
            auto pixels = grabber.getPixels();
            mesh.clear();
            mesh.setMode(OF_PRIMITIVE_POINTS);
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    ofColor cur = pixels.getColor(x, y);
                    float previous_depth = depths[y * width + x];
                    float this_depth = cur.r * 0.28 + cur.g * 0.6 + cur.b * 0.12;
                    float depth = previous_depth * 0.95 + this_depth * 0.05;
                    depths[y * width + x] = depth;
                    if(depth > 20 && depth < 200) {
                            // the alpha value encodes depth, let's remap it to a good depth range
                        float z = ofMap(depth, 0, 255, -300, 300);
                        cur.a = depth;
                        mesh.addColor(cur);
                        ofVec3f pos(width - x - width / 2, y - height / 2, z);
                        mesh.addVertex(pos);
                    }
                }
            }
        }
    }
    void draw() {
        ofBackground(0);
        cam.begin();
        ofScale(1, -1, 1); // make y point down
        shader.begin();
        shader.setUniform1f("focusDistance", focusDistance);
        shader.setUniform1f("aperture", aperture);
        glEnable(GL_POINT_SMOOTH); // makes circular points
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);	// allows per-point size
        mesh.draw();
        shader.end();
        cam.end();
    }
    void keyPressed(int key) {
        if(key == 'f'){
            ofToggleFullscreen();
        }
    }
    
    vector<float> depths;
    int width, height;
    ofVideoGrabber grabber;
    ofEasyCam cam;
    ofMesh mesh;
    
    ofShader shader;
    float focusDistance, aperture;
};


//========================================================================
int main( ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
