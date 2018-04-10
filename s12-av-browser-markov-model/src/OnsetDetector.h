#pragma once
#include "CircularBuffer.h"

class OnsetDetector {

public:
    OnsetDetector () {
        buffer.setSize(32);
        min_samples_per_onset = 5000;
        n_samples_since_onset = 0;
        n_samples = 0;
    }
    
    bool play(double s) {
        buffer.insert(s);
        if(buffer.isFull()) {
            float sum = 1e-10;
            deque<float> buf = buffer.getValues();
            for (int i = 0; i < buf.size(); i++) {
                sum += buf[i] * buf[i];
            }
            float rms = sqrt(sum / (float)buf.size());
            
            // This is the Knuth algorithm for an "online" version of
            // mean/variance.  It means we don't have to keep track of our
            // history and keep averaging, we just do it on the new data
            // coming in without recomputing previous samples.
            n_samples += 1;
            mean_prev = mean;
            mean = mean + (rms - mean) / (float)n_samples;
            meansq = meansq + (rms - mean) * (rms - mean_prev);
            var = meansq / (float)n_samples;
            std = sqrt(var);
            
            if(n_samples_since_onset > min_samples_per_onset)
            {
                // now we see if the current value is outside the mean + variance
                // basic statistics tells us a normally distributed function
                // has a mean and a variance where 97% of the data is explained by
                // 3 standard deviations.  we use this principle here in detecting
                // the the current rms reading is outside this probability
                float min_rms = 0.001;
                if (rms > (mean + 2.0 * std) &&
                    rms > min_rms) {
                    cout << "segment length: " << n_samples_since_onset << endl;
                    n_samples_since_onset = 0;
                    return true;
                }
            }
        }
        n_samples_since_onset++;
        return false;
    }
    

private:
    int                     n_samples, n_samples_since_onset;
    int                     min_samples_per_onset;
    int                     size;
    float                    mean_prev, mean, meansq, var, std;
    CircularBuffer          buffer;
};
