#pragma once

class Recording {
public:
    Recording() {
        should_loop = false;
    }
    ~Recording() {
    
    }
    
    void addAudio(float s) {
        audio.push_back(s);
    }
    
    void addDescriptors(vector<float> s) {
        descriptors.push_back(s);
    }
    
    // return a "pointer" to our audio, means we don't have to copy it,
    // and the "shared_ptr" nonsense is just doing what's called "reference counting"
    // which means it will keep track of how many copies of this are out there,
    // and once it goes to 0 copies, it will free up the memory.  If we didn't
    // use shared_ptr's, we'd lose some performance by always copying a bunch of
    // data, but the functionality would stay exactly the same.
    shared_ptr<vector<float>> getAudio() {
        return make_shared<vector<float>>(audio);
    }
    
    // We can store a lot of MFCC frames, so this is just taking the mean of each
    // individual MFCC feature over time (13 features in total).
    // Recall that the Recording stores many FFT frames, say with a hop size of 1024
    // samples, we have a new FFT every 1024 samples, but our recording might be
    // 44100 samples, or 1 second long... that means we'd have 44100 / 1024 frames
    // of FFTs, and therefore that many MFCC frames.  This collapses all those into
    // just 1 MFCC frame representing the mean MFCC of the segment.
    vector<float> getMeanDescriptor() {
        vector<float> mean_frame;
        int n_features = descriptors[0].size();
        mean_frame.resize(n_features);
        for (int j = 0; j < n_features; j++) {
            mean_frame[j] = 0;
            for (int i = 0; i < descriptors.size(); i++) {
                mean_frame[j] += descriptors[i][j];
            }
        }
        for (int j = 0; j < mean_frame.size(); j++) {
            mean_frame[j] /= (float)descriptors.size();
        }
        return mean_frame;
    }
    
    // convenience method to say how many features are in each descriptor
    // i.e. the number of MFCCs in each MFCC vector
    int getNumberOfFeaturesPerDescriptor() {
        if(descriptors.size())
            return descriptors[0].size();
        else
            return 0;
    }
    
    int getDescriptorsSize() {
        return descriptors.size();
    }
    
    vector<float> getDescriptors(int strategy = 0) {
        // if we don't have any descriptors, return empty vector
        if (descriptors.size() == 0) {
            return vector<float>();
        }
        
        // get the first frame (representing onset's MFCC) + mean frame (mean MFCCs)
        if(strategy == 0) {
            int n_features = getNumberOfFeaturesPerDescriptor();
            vector<float> first_frame, mean_frame, descriptor;
            mean_frame = getMeanDescriptor();
            first_frame = descriptors[0];
            descriptor.resize(n_features * 2);
            for (int j = 0; j < n_features; j++)
                descriptor[j] = first_frame[j];
            for (int j = 0; j < n_features; j++)
                descriptor[j + n_features] = mean_frame[j];
            return descriptor;
        }
        // get just the first frame (could implement other strategies such
        // as delta MFCCs, delta delta MFCCs, etc...!)
        else {
            return descriptors[0];
        }
    }
    
    void reset() {
        audio.resize(0);
    }
    
    float getSample(int i) {
        return audio[i];
    }
    
    int size() {
        return audio.size();
    }
    
    // Simply play functionality... this could be expanded into a granular enginer
    // or subclassed to handle some more clever playback functionality!
    float play(float speed = 1.0) {
        setPosition(position + speed);
        return audio[position];
    }
    
    void setPosition(float p) {
        position = std::max<float>(
            std::min<float>(audio.size() - 1, p),
            0.0);
        if(should_loop) {
            if(position == audio.size() - 1) {
                position = 0.0;
            }
        }
    }
private:
    bool should_loop;
    float position;
    vector<float> audio;
    vector<vector<float>> descriptors;
};

