#pragma once

#include <string>
#include <functional>

namespace std {
	
	void write_file(const char* filename, const string& content);

	std::string read_file(const char * filename);
	
	void read_and_write_file(const char * filename, std::function<std::string(const std::string&)> transform_content);
	
}
