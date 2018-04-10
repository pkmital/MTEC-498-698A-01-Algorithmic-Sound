#pragma once
#include "ofMain.h"
#include "ConstantQ.h"

class Corpus {
public:
    Corpus() {
    
    }
    
    void addRecording(shared_ptr<Recording> r) {
        recordings.push_back(r);
    }
    
    int size() {
        return recordings.size();
    }
    
    // we make this function virtual because or VisualCorpus will override it
    virtual void analyze() {
        // Analyze every recording
        for (int i = 0; i < recordings.size(); i++) {
            // Create the FFT/MFCC analyzers for each recording
            // (so it resets its sample history)
            maxiFFT fft;
            maxiMFCCAnalyser<float> mfcc;
            int fft_size = 4096;
            int window_size = fft_size / 2;
            int hop_size = fft_size / 4;
            int n_mfcc = 13;
            int n_mel = 42;
            int sr = 44100;
            int min_freq = 20;
            int max_freq = sr / 2;
            int n_bins = fft_size / 2;
            fft.setup(fft_size, window_size, hop_size);
            mfcc.setup(n_bins, n_mel, n_mfcc, min_freq, max_freq, sr);
            
            // Get the next recording
            shared_ptr<Recording> r = recordings[i];
            vector<float> previous_mel_values;
            for (int sample_i = 0; sample_i < r->size(); sample_i++) {
                // give each sample to the FFT process.  It returns true when
                // there is a new FFT available
                if(fft.process(r->getSample(i))) {
                    // create some storage for MELs
                    vector<float> mel_values, descriptor;
                    mel_values.resize(n_mel);
                    descriptor.resize(n_mel + n_mel);
                    // Pass in our FFT magnitudes and storage which will
                    // get filled up w/ MELs
                    mfcc.mel(fft.magnitudes, &(mel_values[0]));
                    
                    if(previous_mel_values.size()) {
                        for(int mel_i = 0; mel_i < n_mel; mel_i++) {
                            descriptor[mel_i] = mel_values[mel_i];
                            descriptor[mel_i + n_mel] = mel_values[mel_i] - previous_mel_values[mel_i];
                        }
                        // Append the MELs + delta MELs to our recording object
                        r->addDescriptors(descriptor);
                    }
                    else
                        previous_mel_values.resize(n_mel);
                    
                    previous_mel_values = mel_values;
                }
            }
            
            // ask the recording for their description now that we've analyzed it
            // this can be one of a few types of strategies, which try to combine
            // all the MFCCs we've added
            if(r->getDescriptorsSize())
                descriptors.push_back(r->getDescriptors());
        }
        
        // This will find the mean and std descriptor, then z-normalize the descriptors
        centerDescriptors();
    }
    
// We make this protected so our "VisualCorpus" which extends us gets access to
// these things.
protected:
    int getDescriptorSize() {
        if(descriptors.size()) {
            return descriptors[0].size();
        }
        else {
            return 0;
        }
    }

    // we find the mean and standard deviation of all of our descriptors here
    // this lets us Z-Normalize our descriptors, and is often a much better
    // representation for any clustering/machine learning/etc..
    void buildStatistics() {
        int n_features = getDescriptorSize();
        if(n_features == 0) {
            return vector<float>();
        }

        mean_descriptor.resize(n_features);
        std_descriptor.resize(n_features);
        for(int j = 0; j < n_features; j++) {
            mean_descriptor[j] = 0.0;
            for(auto descriptor : descriptors) {
                mean_descriptor[j] += descriptor[j];
            }
            mean_descriptor[j] /= descriptors.size();
            
            std_descriptor[j] = 0.0;
            if (descriptors.size() > 1) {
                for(auto descriptor : descriptors) {
                    float d = (descriptor[j] - mean_descriptor[j]);
                    std_descriptor[j] += (d * d);
                }
                std_descriptor[j] = sqrtf(std_descriptor[j] + 1e-12) / (descriptors.size() - 1);
            }
        }
    }

    // this is where we actually center our descriptors (i.e. z-normalize)
    void centerDescriptors() {
        // first find the mean and standard deviation of all our descriptors
        buildStatistics();
        int n_features = getDescriptorSize();
        
        // now center each descriptor
        for(auto descriptor : descriptors) {
            vector<float> normalized;
            normalized.resize(n_features);
            for(int j = 0; j < n_features; j++) {
                // centering involves removing the mean
                // then unscaling the deviation
                // the data should be mostly at 0, +/- 3 after this process.
                normalized[j] = (descriptor[j] - mean_descriptor[j]) / std_descriptor[j];
            }
            normalized_descriptors.push_back(normalized);
        }
    }

    vector<shared_ptr<Recording>> recordings;
    vector<vector<float>> descriptors;
    vector<vector<float>> normalized_descriptors;
    vector<float> mean_descriptor, std_descriptor;
    vector<vector<double>> positions;
    ofxTSNE tsne;

};

