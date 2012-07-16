
#pragma once

// unique id generator -- singleton class
class UID
{
public:
    static int create() {
        int id = generator;
        generator++;
        return id;
    }
    
private:
    static int generator;
    
private:
    UID() {}
};