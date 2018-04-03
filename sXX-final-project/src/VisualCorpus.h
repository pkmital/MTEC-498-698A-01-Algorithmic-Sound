#pragma once
#include "ofMain.h"
#include "Corpus.h"

// We extend the Corpus class so that we can abstract the drawing part of stuff
// but will still think of ourselves as a Corpus type
class VisualCorpus : public Corpus {
public:

    VisualCorpus() {
    
    }

    void setupCanvas(int width = 1024, int height = 768, int depth = 500)  {
        w = width;
        h = height;
        z = depth;
    }

    // we make this function virtual because our AudioVisualCorpus will override it
    virtual void analyze() {
        // this is how we call the base class, Corpus, which has the same name
        // function.
        Corpus::analyze();

        positions = tsne.run(normalized_descriptors, 3, 50, 1.0);
        
        // Now we build up a vertex buffer of our TSNE points
        mesh.setMode(OF_PRIMITIVE_POINTS);
        for (auto p_i : positions) {
            ofVec3f v((p_i[0] - 0.5) * w,
                      (p_i[1] - 0.5) * h,
                      0);
            mesh.addVertex(v);
            ofColor c;
            c.setHsb(p_i[2] * 255.0, 200, 255);
            mesh.addColor(c);
        }
        glPointSize(5.0);
        ofEnableAntiAliasing();
    }

    void draw() {
        ofEnableAlphaBlending();
        ofSetColor(255, 200);
        // Draw all our vertices
        mesh.draw();
        ofDisableAlphaBlending();
        
        // Get the position from our mesh using the idx
        ofVec2f position = mesh.getVertex(nearest_idx);
        
        // Draw a box at the location of the selected vertex
        ofNoFill();
        ofDrawBox(position.x, position.y, 0.0, 10);
        /*
        // Draw a set of axes at the location
        ofSetColor(200, 100, 100);
        ofDrawLine(position.x, position.y, -100,
            position.x, position.y, 100);
        ofSetColor(100, 200, 100);
        ofDrawLine(position.x - 100, position.y, 0,
            position.x + 100, position.y, 0);
        ofSetColor(100, 100, 200);
        ofDrawLine(position.x, position.y - 100, 0,
            position.x, position.y + 100, 0);
        */
        ofFill();
    }
    
    int mouseMoved(int x, int y, const ofCamera& cam) {
        float nearest_dist = HUGE_VALF;
        nearest_idx = 0;
        ofVec3f mouse(x, y, 0);
        for (int i = 0; i < mesh.getNumVertices(); i++) {
            // we use the modelview projection matrix to project the vertices
            // based on the cam properties, allowing us to do nearest distance
            // to screen coordinates regardless of the camera view
            ofVec3f x = cam.worldToScreen(mesh.getVertices()[i]);
            float distance = mouse.squareDistance(x);
            if(distance < nearest_dist) {
                nearest_dist = distance;
                nearest_idx = i;
            }
        }
        return nearest_idx;
    }

protected:
    ofVboMesh mesh;
    int w, h, z, n_dim;
    int nearest_idx;
};

