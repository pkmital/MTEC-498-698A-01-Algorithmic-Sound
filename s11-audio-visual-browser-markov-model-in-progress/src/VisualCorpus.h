#pragma once
#include "ofMain.h"
#include "Corpus.h"
#include "ofxLearn.h"

// We extend the Corpus class so that we can abstract the drawing part of stuff
// but will still think of ourselves as a Corpus type
class VisualCorpus : public Corpus {
public:

    VisualCorpus() {
        use_tsne = false;
        n_clusters = 50;
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
        
        if(use_tsne) {
            buildTSNE();
        } else {
            buildPCA();
        }
        
        buildKMeans();
        
        buildTransitionMatrix();
        
        // Now we build up a vertex buffer of our projected points
        mesh.setMode(OF_PRIMITIVE_POINTS);
        int i = 0;
        for (auto p_i : positions) {
            cout << p_i[0] << ", " << p_i[1] << endl;
            ofVec3f v((p_i[0] - 0.5) * w,
                      (p_i[1] - 0.5) * h,
                      0);
            mesh.addVertex(v);
            ofColor c;
            c.setHsb(clusters[i] / (float)(n_clusters - 1) * 255.0, 200, 255);
            mesh.addColor(c);
            i++;
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
        // we want to find the nearest vertex to our mouse location
        // start off with a huge distance, we'll find a better one
        float nearest_dist = HUGE_VALF;
        // assume 0 is the best idx
        nearest_idx = 0;
        // grab the current location
        ofVec3f mouse(x, y, 0);
        // now iterate across every vertex, and calculate distance
        for (int i = 0; i < mesh.getNumVertices(); i++) {
            // we use the modelview projection matrix to project the vertices
            // based on the cam properties, allowing us to do nearest distance
            // to screen coordinates regardless of the camera view
            ofVec3f x = cam.worldToScreen(mesh.getVertices()[i]);
            // get the distance
            float distance = mouse.squareDistance(x);
            // store it if it is the best distance (closest to our mouse)
            if(distance < nearest_dist) {
                nearest_dist = distance;
                nearest_idx = i;
            }
        }
        return nearest_idx;
    }

protected:

    void buildKMeans() {
        ofxLearnKMeans kmeans;
        for(auto descriptor : normalized_descriptors) {
            int n_features = descriptor.size();
            vector<double> d;
            d.resize(n_features);
            for(int i = 0; i < n_features; i++) {
                d[i] = descriptor[i];
            }
            kmeans.addSample(d);
        }
        kmeans.setNumClusters(n_clusters);
        kmeans.train();
        clusters = kmeans.getClusters();
    }

    void buildTSNE() {
        // how many dimensions to reduce our descriptors to
        int n_dim = 3;
        // see: https://distill.pub/2016/misread-tsne/ for explanation of perplexity
        int perplexity = 50;
        
        // reduce our descriptors down to n_dim
        // allowing us to visualize them in 2D
        // normalized_descriptors is part of the parent class, Corpus
        positions = tsne.run(normalized_descriptors, n_dim, perplexity, 1.0);
    }
    
    void buildPCA() {
        ofxLearnPCA pca;
        for(auto descriptor : normalized_descriptors) {
            int n_features = descriptor.size();
            vector<double> d;
            d.resize(n_features);
            for(int i = 0; i < n_features; i++) {
                d[i] = descriptor[i];
            }
            pca.addSample(d);
        }
        pca.pca(3);
        positions = pca.getProjectedSamples();
        float min_val_x = HUGE_VALF, min_val_y = HUGE_VALF, min_val_z = HUGE_VALF;
        float max_val_x = -HUGE_VALF, max_val_y = -HUGE_VALF, max_val_z = -HUGE_VALF;
        for(int i = 0; i < positions.size(); i ++) {
            if (positions[i][0] < min_val_x)
                min_val_x = positions[i][0];
            if (positions[i][0] > max_val_x)
                max_val_x = positions[i][0];
            
            if (positions[i][1] < min_val_y)
                min_val_y = positions[i][1];
            if (positions[i][1] > max_val_y)
                max_val_y = positions[i][1];
            
            if (positions[i][2] < min_val_z)
                min_val_z = positions[i][2];
            if (positions[i][2] > max_val_z)
                max_val_z = positions[i][2];
        }
        
        for(int i = 0; i < positions.size(); i ++) {
            positions[i][0] = (positions[i][0] - min_val_x) / (max_val_x - min_val_x);
            positions[i][1] = (positions[i][1] - min_val_y) / (max_val_y - min_val_y);
            positions[i][2] = (positions[i][2] - min_val_z) / (max_val_z - min_val_z);
        }
    }
    
    void buildTransitionMatrix(){
        // initialize empty transition matrix to n_clusters x n_clusters
        transition_matrix.resize(n_clusters);
        cdf.resize(n_clusters);
        for(int i = 0; i < n_clusters; i++) {
            transition_matrix[i].resize(n_clusters);
            cdf[i].resize(n_clusters);
            for(int j = 0; j < n_clusters; j++) {
                // initialize to non-zero (play with this !)
                transition_matrix[i][j] = 0.1;
                
                cdf[i][j] = 0.0;
            }
        }
        
        // build up the transition matrix (unnormalized)
        int n_recordings = clusters.size();
        for(int i = 1; i < n_recordings; i++) {
            transition_matrix[clusters[i - 1]][clusters[i]]++;
        }
        
        // normalize the transition_matrix
        for(int i = 0; i < n_clusters; i++) {
            // get total value of this state's transitions
            float sum = 1e-12;
            for(int j = 0; j < n_clusters; j++) {
                sum += transition_matrix[i][j];
            }
            // normalize this state's transitions by sum, so that it becomes a
            // probability density function (PDF)
            for(int j = 0; j < n_clusters; j++) {
                transition_matrix[i][j] /= sum;
            }
            // build CDF (cumulative distribution function)
            cdf[i][0] = transition_matrix[i][0];
            for(int j = 1; j < n_clusters; j++) {
                cdf[i][j] = cdf[i][j - 1] + transition_matrix[i][j];
            }
        }
    }

    vector<vector<double>> transition_matrix;
    vector<vector<double>> cdf;
    vector<int> clusters;
    ofVboMesh mesh;
    int w, h, z, n_dim, n_clusters;
    int nearest_idx;
    bool use_tsne;
};

