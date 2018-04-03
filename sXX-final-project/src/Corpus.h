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
            int n_mfcc = 13;
            fft.setup(4096, 2048, 1024);
            mfcc.setup(2048, 128, n_mfcc, 20, 12000, 44100);
            
            // Get the next recording
            shared_ptr<Recording> r = recordings[i];
            vector<float> previous_mfcc_values;
            for (int sample_i = 0; sample_i < r->size(); sample_i++) {
                // give each sample to the FFT process.  It returns true when
                // there is a new FFT available
                if(fft.process(r->getSample(i))) {
                    // create some storage for MFCCs
                    vector<float> mfcc_values, descriptor;
                    mfcc_values.resize(n_mfcc);
                    descriptor.resize(n_mfcc + n_mfcc);
                    // Pass in our FFT magnitudes and storage which will
                    // get filled up w/ MFCCs
                    mfcc.mfcc(fft.magnitudes, &(mfcc_values[0]));
                    
                    if(previous_mfcc_values.size()) {
                        for(int mfcc_i = 0; mfcc_i < n_mfcc; mfcc_i++) {
                            descriptor[mfcc_i] = mfcc_values[mfcc_i];
                            descriptor[mfcc_i + n_mfcc] = mfcc_values[mfcc_i] - previous_mfcc_values[mfcc_i];
                        }
                        // Append the MFCCs + delta MFCCs to our recording object
                        r->addDescriptors(descriptor);
                        previous_mfcc_values = mfcc_values;
                    }
                    else
                        previous_mfcc_values.resize(n_mfcc);
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

    void centerDescriptors() {
        buildStatistics();
        int n_features = getDescriptorSize();
        for(auto descriptor : descriptors) {
            vector<float> normalized;
            normalized.resize(n_features);
            for(int j = 0; j < n_features; j++) {
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

