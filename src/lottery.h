
#pragma once

#include <vector>
#include <cstdlib>
using namespace std;

// support draw without replacement
class Lottery
{
public:
    Lottery(int size_)
    {
        reset(size_);
    }
    
    void reset(int size_)
    {
        size = left = size_;
        balls.resize(size);
        selected.resize(size);
        for(int i=0; i<size; i++) {
            balls[i] = i;
            selected[i] = false;
        }
    }
    
    bool isDrawn(int i) const {
        return selected[i];
    }
    
    int draw()
    {
        if(left == 0)
            return -1;
        int slot = rand() % left;
        int number = balls[slot];
        selected[number] = true;
        // swap
        balls[slot] = balls[left];
        balls[left] = number;
        // forget about the drawn ball
        left--;
        return number;
    }
    
    void drawN(vector<int> &draws, int n)
    {
        draws.resize(n);
        for(int i=0; i<n; i++)
            draws[i] = draw();
    }
    
    void drawN(int n)
    {
        for(int i=0; i<n; i++)
            draw();
    }
    
private:
    int size;
    int left;
    vector<int> balls;
    vector<bool> selected;
};