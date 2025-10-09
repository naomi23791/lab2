#pragma once
#include <iostream>

class redirect_output_wrapper {
    std::ostream& stream;
    std::streambuf* old_buf;
    
public:
    // Constructor: saves the original buffer
    redirect_output_wrapper(std::ostream& src) 
        : stream(src), old_buf(src.rdbuf()) {}
    
    // Destructor: restores the original buffer
    ~redirect_output_wrapper() {
        stream.rdbuf(old_buf);
    }
    
    // Redirects output to a new stream
    void redirect(std::ostream& dest) {
        stream.rdbuf(dest.rdbuf());
    }
};