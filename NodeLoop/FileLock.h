#pragma once

#include <string>
#include <functional>

namespace std {
	
	void write_file_with_lock(const char* filename);

	std::string read_file_with_lock(const char * filename);
	
	void read_and_write_file_with_lock(const char * filename, std::function<std::string(const std::string&)> transform_content);
	
}
