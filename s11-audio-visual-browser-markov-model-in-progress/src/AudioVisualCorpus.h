#pragma once
#include "VisualCorpus.h"


class AudioVisualCorpus : public VisualCorpus {
public:
    AudioVisualCorpus() {
        previous_grain = 0;
    }
    
    // After building the corpus, we analyze it.  Each parent class of AudioVisualCorpus has it's own analyze method that will get called in a chain.
    virtual void analyze()  {
        VisualCorpus::analyze();
        for (auto r : recordings) {
            grains.push_back(r);
        }
    }
    
    // This is our really simple audio engine which will just keep track of which
    // recordings are being selected by the mouse, and then trigger them for playback
    // It will also window in and window out, so we avoid clicks.
    void audioOut(float *buf, int bufferSize, int ch) {
        float speed = 1.0;
        
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
        
        // Just check if the current grain is the first time we are playing it.
        bool should_window = false;
        
        // Simple flag to check whether the VisualCorpus has reassigned the
        // "nearest_idx" based on the mouseMoved function.  Then we need to update
        // the associated grain and stop playing the current one.
        if(previous_grain != nearest_idx) {
            ending_grains.push_back(grains[previous_grain]);
            grains[nearest_idx]->setPosition(0.0);
            previous_grain = nearest_idx;
            should_window = true;
        }
        
        // Now we fill our audio buffer with our grains
        for(int i = 0; i < bufferSize; i++) {
            buf[i] = 0.0;
            if (should_window) {
                buf[i] += in_window[i] * grains[nearest_idx]->play(speed);
            }
            else {
               buf[i] += grains[nearest_idx]->play(speed);
            }
            for(auto g : ending_grains) {
                buf[i] += out_window[i] * g->play(speed);
            }
        }
       ending_grains.clear();
    }
    
    
private:
    // Nice window to avoid clicks
    float hann(int windowLength, int windowPos) {
        return 0.5 * (1.0 - cos((2.0 * PI * windowPos) / (windowLength - 1)));
    }

    int previous_grain;
    vector<float> in_window, out_window;
    vector<shared_ptr<Recording>> grains;
    vector<shared_ptr<Recording>> ending_grains;
};
