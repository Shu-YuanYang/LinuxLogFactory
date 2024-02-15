#include "FileLock.h"
#include <iostream>
#include <fstream>
#include <streambuf>




namespace std {
	
 

	void write_file(const char* filename, const string& content) {
		std::ofstream out(filename);
		out << content;
		out.close();
	}

    void read_and_write_file(const char* filename, std::function<std::string(const std::string&)> transform_content) {

        // 2. transform content:
        std::string new_content = transform_content("");
        
    }
	

	std::string read_file(const char* filename) {
		std::ifstream f(filename);
		std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		f.close();
		return str;
	}

}
