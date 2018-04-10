#include "ofApp.h"
#include "ofxMaxim.h"

// beat repeat
// hidden markov models
// SVD for real-time clustering
// beat tracking / tempo estimation
// quantize time / when you transition to get fixed tempo
// exploring other corpora
// polyphonic pitch detection
// choosing more than 1 grain, perhaps using NN, or by using the most similar sound in that cluster to the previously played sound
// chromagram for pitch-based features

//--------------------------------------------------------------
void ofApp::setup(){
    width = 1200;
    height = 600;
    ofSetWindowShape(width, height);
    
    corpus.setupCanvas(width, height);
    
    sample.load(ofToDataPath("mtume-w-synth.wav"));
    long sample_length = sample.length;
    shared_ptr<Recording> r = make_shared<Recording>();
    for (int i = 0; i < sample_length; i++) {
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
            cout << "Percent complete: " << i / (float)sample_length * 100.0 << ", Corpus size: " << corpus.size() << endl;
        }
    }
    cout << "Finished building corpus.  Analyzing..." << endl;
    corpus.analyze();
    
    maxiSettings::setup(44100, 1, 128);
    ofSoundStreamSetup(1, 0, 44100, 128, 3);
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

void ofApp::keyPressed(int k) {
    if(k == ' ') {
        if (mode == AudioVisualCorpus::SYNTH_MOUSE)
            mode = AudioVisualCorpus::SYNTH_MARKOV;
        else
            mode = AudioVisualCorpus::SYNTH_MOUSE;
        corpus.setMode(mode);
    }
    else if(k == 'g') {
        corpus.toggleGrid();
    }
        
}
