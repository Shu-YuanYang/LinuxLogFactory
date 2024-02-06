#pragma once

#include <string>

namespace std {
	
	void write_file_with_lock(const char* filename);

	std::string read_file_with_lock(const char * filename);
	
}
