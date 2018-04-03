#pragma once

class CircularBuffer {

public:
    CircularBuffer() {
    
    }
    
    void setSize(int s) {
        n_size = s;
    }
    
    void insert(float v) {
        buffer.push_front(v);
        while(buffer.size() > n_size) {
            buffer.pop_back();
        }
    }
    
    deque<float> getValues() {
        return buffer;
    }
    
    int size() {
        return buffer.size();
    }
    
    bool isFull() {
        return size() == n_size;
    }
    
private:
    deque<float> buffer;
    int n_size;
};
