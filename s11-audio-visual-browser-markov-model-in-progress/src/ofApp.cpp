#include "ofApp.h"
#include "ofxMaxim.h"

//--------------------------------------------------------------
void ofApp::setup(){
    width = 1200;
    height = 600;
    ofSetWindowShape(width, height);
    
    corpus.setupCanvas(width, height);
    
    sample.load(ofToDataPath("swedlow.wav"));
    long sample_length = sample.length;
    shared_ptr<Recording> r = make_shared<Recording>();
    for (int i = 0; i < sample_length * 0.15; i++) {
        // get latest sample from maxiSample player
        double s = sample.play();
        // add sample to Recording
        r->addAudio(s);
        // ask the onset detector if it is an onset
        if(detector.play(s)) {
            // got onset, add the Recording to the corpus
            corpus.addRecording(r);
            // reset the recording so we can start a new one
            r = make_shared<Recording>();
            cout << "Corpus size: " << corpus.size() << endl;
        }
    }
    cout << "Finished building corpus.  Analyzing..." << endl;
    corpus.analyze();
    
    maxiSettings::setup(44100, 1, 256);
    ofSoundStreamSetup(1, 0, 44100, 256, 3);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(30);
    cam.begin();
    corpus.draw();
    cam.end();
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    corpus.mouseMoved(x, y, cam);
}

//--------------------------------------------------------------
void ofApp::audioOut(float *buffer, int bufferSize, int ch) {
    corpus.audioOut(buffer, bufferSize, ch);
}
