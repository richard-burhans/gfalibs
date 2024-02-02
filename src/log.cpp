#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include "bed.h"
#include "struct.h"
#include "functions.h"
#include "global.h"
#include "log.h"

void Log::newlines(unsigned int n) { // add new lines
    
    if(verbose_flag)
        std::cerr<<std::string(n, '\n');
    
}

void Log::verbose(std::string msg, bool overwrite) { // verbose decorated output
    
    if(verbose_flag) {
        
        if (overwrite) {std::cerr << "\r" << msg << "      "; return;};
        
        std::cerr << msg << " (done in " << std::to_string(elapsedTime()) << " s).\n"; // if you don't cast double to string it will mess up all file output!
        
        elapsedTime();
        
    }
}

void Log::add(std::string msg) { // verbose decorated output
    
    if(verbose_flag) {
    
        log += msg + " (done in " + std::to_string(elapsedTime()) + " s).\n"; // if you don't cast double to string it will mess up all file output!
        
        elapsedTime();
        
    }
}

void Log::print() {
    
    std::cerr << log;
    log.clear();
    
}

void Log::setId (unsigned int i) {
    
    jobId = i;
    
}
