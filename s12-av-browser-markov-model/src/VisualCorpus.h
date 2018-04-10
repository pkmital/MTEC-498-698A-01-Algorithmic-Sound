#pragma once
#include "ofMain.h"
#include "Corpus.h"
#include "ofxLearn.h"
#include "ofxAssignment.h"

// We extend the Corpus class so that we can abstract the drawing part of stuff
// but will still think of ourselves as a Corpus type
class VisualCorpus : public Corpus {
public:

    VisualCorpus() {
        use_tsne = true;
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
        
        // make sure we don't try and ask for more clusters than we have recordings
        int grid_size = sqrt(recordings.size());
        vector<ofVec2f> grid = makeGrid(grid_size, grid_size);
        vector<ofVec2f> pts;
        for (int i = 0; i < grid_size * grid_size; i++) {
            ofVec2f v((positions[i][0] - 0.5) * w,
                      (positions[i][1] - 0.5) * h);
            pts.push_back(v);
        }
        vector<ofVec2f> grid_pts = assign.match<ofVec2f>(
            pts, grid);
        
        n_clusters = sqrt(recordings.size());
        buildKMeans();
        buildTransitionMatrix();
        
        // Now we build up a vertex buffer of our projected points
        grid_mesh.setMode(OF_PRIMITIVE_POINTS);
        int i = 0;
        for (auto p_i : grid_pts) {
            ofVec3f v((p_i[0] - 0.5) * w,
                      (p_i[1] - 0.5) * h);
            grid_mesh.addVertex(v);
            ofColor c;
            c.setHsb(clusters[i] / (float)(n_clusters - 1) * 255.0, 200, 255);
            grid_mesh.addColor(c);
            i++;
        }
        
        mesh.setMode(OF_PRIMITIVE_POINTS);
        i = 0;
        for (auto p_i : positions) {
            ofVec3f v((p_i[0] - 0.5) * w,
                      (p_i[1] - 0.5) * h);
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
        if(use_grid)
            grid_mesh.draw();
        else
            mesh.draw();
        ofDisableAlphaBlending();
        
        // Get the position from our mesh using the idx
        ofVec2f position;
        if(use_grid)
            position = grid_mesh.getVertex(nearest_idx);
        else
            position = mesh.getVertex(nearest_idx);
        
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
    
    virtual int mouseMoved(int x, int y, const ofCamera& cam) {
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
            ofVec3f x;
            if(use_grid) {
                x = cam.worldToScreen(grid_mesh.getVertices()[i]);
            }
            else {
                x = cam.worldToScreen(mesh.getVertices()[i]);
            }
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
    
    void toggleGrid() {
        use_grid = !use_grid;
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
        
        // also keep track of which idx has which cluster
        cluster_lists.resize(n_clusters);
        for(int i = 0; i < clusters.size(); i++) {
            cluster_lists[clusters[i]].push_back(i);
        }
        cout << "Found the following clusters: " << endl;
        printMatrix(cluster_lists);
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
        initializeMatrix<double>(transition_matrix, n_clusters, n_clusters, 0.01);
        
        // also initialize our cdf as well
        initializeMatrix<double>(cdf, n_clusters, n_clusters);
        
        // build up the transition matrix (unnormalized) of observations of
        // pairs from cluster(i-1) and cluster(i), knowing that clusters
        // is a vector stating which cluster a segment/recording is in, and that
        // each segment happens in chronological order.
        // in other words, this loops goes through the entire recording, one
        // segment at a time, looking at the transitions between each's segments
        // label: i.e. which cluster that segment ended up being labeled as.
        // this is really the only "machine learning" part and it is a one line
        // for loop!
        int n_recordings = clusters.size();
        for(int i = 1; i < n_recordings; i++) {
            transition_matrix[clusters[i - 1]][clusters[i]]++;
        }
        
        // print the transition matrix (for debug)
        cout << "transition_matrix:" << endl;
        printMatrix<double>(transition_matrix);
        
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
        
        // print the cdf matrix (for debug)
        cout << "cdf:" << endl;
        printMatrix<double>(cdf);
    }
    
    template <typename T>
    void initializeMatrix(vector<vector<T>> &mat, size_t rows, size_t cols, T val=0) {
        mat.resize(rows);
        for(int i = 0; i < mat.size(); i++) {
            mat[i].resize(cols);
            for(int j = 0; j < mat[i].size(); j++) {
                mat[i][j] = val;
            }
        }
    }
    
    template <typename T>
    void printMatrix(const vector<vector<T>> &mat) {
        cout << "rows: " << mat.size() << ", cols: " << mat.begin()->size() << endl;
        for(int i = 0; i < mat.size(); i++) {
            for(int j = 0; j < mat[i].size(); j++) {
                cout << mat[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    vector<vector<double>> transition_matrix;
    vector<vector<double>> cdf;
    vector<int> clusters;
    vector<vector<int>> cluster_lists;
    ofVboMesh grid_mesh, mesh;
    int w, h, z, n_dim, n_clusters;
    int nearest_idx;
    bool use_tsne, use_grid;
    ofxAssignment assign;
};

