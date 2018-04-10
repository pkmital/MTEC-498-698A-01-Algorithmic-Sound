#pragma once
#include "VisualCorpus.h"


class AudioVisualCorpus : public VisualCorpus {
public:
    enum SYNTH_MODE {
        SYNTH_MARKOV,
        SYNTH_MOUSE
    };
    
    AudioVisualCorpus() {
        synth_mode = SYNTH_MARKOV;
        previous_grain = -1;
        current_grain = -1;
        previous_state = current_state = 0;
        samples_since_onset = 0;
        should_window = false;
    }
    
    // After building the corpus, we analyze it.  Each parent class of AudioVisualCorpus has it's own analyze method that will get called in a chain.
    virtual void analyze()  {
        // call our parent class to analyze, which will call its parent class, Corpus, to analyze.  That will create the segments and also all the points for display.  It also runs KMeans and assigns labels to each point.
        VisualCorpus::analyze();
        
        // Now we can use the labels from KMeans and the built transition matrix to sample new grains.
        if(synth_mode == SYNTH_MARKOV) {
            sampleGrainFromCurrentState();
        }
    }
    
    void updateGrainsForMouse() {
        // check if we need to update the associated grain and stop playing the current one.  the mouseMoved function will change the current_grain whenever the user moves to a new grain, so this check will become true only when they pick a new grain or reselect the current one (which ends and gets assigned -1 eventually).
        if(previous_grain != current_grain) {
            if(current_grain >= 0) {
                // fade out the current playing grain
                ending_grains.push_back(recordings[current_grain]);
                
                // reset the new grain to play to the start position
                recordings[current_grain]->setPosition(0.0);
                
                // tell our audio engine that it should window this new grain
                should_window = true;
                
                // and we'll just keep track of how many samples it has been since an onset
                samples_since_onset = 0;
            }
            previous_grain = current_grain;
        }
        
        // if the current grain is about to end, let's apply the out window to avoid any clicks and set the grain to -1
        if(recordings.size() && current_grain >= 0 && recordings[current_grain]->willFinish(out_window.size())) {
            ending_grains.push_back(recordings[current_grain]);
            current_grain = -1;
        }
    }
    
    void sampleNextState() {
        float p = (rand() % 100) / 100.0f;
        previous_state = current_state;
        current_state = 0;
        while(cdf[previous_state][current_state] <= p) {
            current_state++;
        }
    }
    
    void sampleGrainFromCurrentState() {
        float p = (rand() % 100) / 100.0f;
        int n_grains_in_state = cluster_lists[current_state].size();
        float p_per_grain = 1.0 / (float)n_grains_in_state;
        int grain_i = 0;
        while((grain_i + 1) * p_per_grain <= p) {
            grain_i++;
        }
        current_grain = cluster_lists[current_state][grain_i];
    }
    
    void updateGrainsForMarkov() {
        
        if(recordings.size() && current_grain >= 0 && recordings[current_grain]->willFinish(out_window.size())) {
            previous_grain = current_grain;
            // select a new one using the probability of our transition matrix of current state
            sampleNextState();
            sampleGrainFromCurrentState();
            if(current_grain != previous_grain)
            {
                ending_grains.push_back(recordings[current_grain]);
                recordings[current_grain]->setPosition(0.0);
                should_window = true;
                samples_since_onset = 0;
            }
            nearest_idx = current_grain;
            cout << current_grain << ", " << previous_grain << endl;
        }
    }
    
    void updateGrains() {
        if(synth_mode == SYNTH_MOUSE) {
            updateGrainsForMouse();
        }
        else {
            updateGrainsForMarkov();
        }
    }
    
    void allocateWindows(int bufferSize) {
        // If we haven't allocated our "in" window, let's allocate it
        // The "in" window will be used whenever a new sample is triggered for playback, and we ramp in the signal to avoid any clicks.
        if(in_window.size() != bufferSize) {
            for(int i = 0; i < bufferSize; i++) {
                in_window.push_back(hann(bufferSize * 2, i));
            }
        }
        
        // If we haven't allocated our window, let's allocate it
        // The "out" window will be used whenever we request to stop playing a sample, and we ramp out the signal to avoid any clicks.
        if(out_window.size() != bufferSize) {
            for(int i = 0; i < bufferSize; i++) {
                out_window.push_back(hann(bufferSize * 2, i + bufferSize));
            }
        }
    }
    // This is our really simple audio engine which will just keep track of which
    // recordings are being selected by the mouse, and then trigger them for playback
    // It will also window in and window out, so we avoid clicks.
    void audioOut(float *buf, int bufferSize, int ch) {
        float speed = 1.0;
        allocateWindows(bufferSize);
        // Now we fill our audio buffer with our grains
        for(int i = 0; i < bufferSize; i++) {
            updateGrains();
            buf[i] = 0.0;
            if(current_grain >= 0) {
                if (should_window) {
                    buf[i] += in_window[samples_since_onset] * recordings[current_grain]->play(speed);
                    for(auto g : ending_grains) {
                        buf[i] += out_window[samples_since_onset] * g->play(speed);
                    }
                }
                else {
                   buf[i] += recordings[current_grain]->play(speed);
                }
            }
            samples_since_onset++;
            if(samples_since_onset >= out_window.size())
                should_window = false;
        }
       ending_grains.clear();
    }
    
    void setMode(SYNTH_MODE m) {
        synth_mode = m;
        if(synth_mode == SYNTH_MARKOV) {
            sampleGrainFromCurrentState();
        }
    }
    
    SYNTH_MODE getMode() {
        return synth_mode;
    }
    
    virtual int mouseMoved(int x, int y, const ofCamera& cam) {
        VisualCorpus::mouseMoved(x, y, cam);
        if(synth_mode == SYNTH_MOUSE)
            current_grain = nearest_idx;
        else
            nearest_idx = current_grain;
    }
    
    
private:
    // Nice window to avoid clicks
    float hann(int windowLength, int windowPos) {
        return 0.5 * (1.0 - cos((2.0 * PI * windowPos) / (windowLength - 1)));
    }
    
    SYNTH_MODE synth_mode;
    int previous_state, current_state, current_grain, previous_grain;
    int samples_since_onset;
    bool should_window;
    vector<float> in_window, out_window;
    vector<shared_ptr<Recording>> ending_grains;
};
