// code copied and modified from https://voyager.deanza.edu/~perry/lock.html
#include "FileLock.h"
#include <sys/types.h>   /***********  Write Lock Setter  *******/
#include <unistd.h> 
#include <fcntl.h>

#include <iostream>


namespace std {
	
 

	void write_file_with_lock(const char* filename) {
        int fd;
        flock lock, savelock;

        fd = open(filename, O_RDWR);

        //std::cout << F_WRLCK << " " << F_RDLCK << std::endl;
        lock.l_type = F_WRLCK; // test for any lock on any part of the file
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        savelock = lock;
        fcntl(fd, F_GETLK, &lock); // overwrites lock structure with preventors

        //std::cout << lock.l_type << std::endl;
        if (lock.l_type == F_WRLCK) {
                std::string err_message = "Process " + std::to_string(lock.l_pid) + " has a write lock already!";
		std::cout << err_message << std::endl;
                //close(fd);
		throw err_message;
		//exit(1);
        }

        if (lock.l_type == F_RDLCK) {
		std::string err_message = "Process " + std::to_string(lock.l_pid) + " has a read lock already!";
                std::cout << err_message << std::endl;
                //close(fd);
		throw err_message;
		//exit(1);
        }

        fcntl(fd, F_SETLK, &savelock);
       	//std::cout << savelock.l_type << std::endl;

		close(fd);
	}

    void read_and_write_file_with_lock(const char* filename, std::function<std::string(const std::string&)> transform_content) {
        
        int fd;
        flock lock, savelock;

        fd = open(filename, O_RDWR);

        //std::cout << F_WRLCK << " " << F_RDLCK << std::endl;
        lock.l_type = F_WRLCK; // test for any lock on any part of the file
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        savelock = lock;
        fcntl(fd, F_GETLK, &lock); // overwrites lock structure with preventors

        //std::cout << lock.l_type << std::endl;
        if (lock.l_type == F_WRLCK) {
            std::string err_message = "Process " + std::to_string(lock.l_pid) + " has a write lock already!";
            std::cout << err_message << std::endl;
            //close(fd);
            throw err_message;
            //exit(1);
        }

        if (lock.l_type == F_RDLCK) {
            std::string err_message = "Process " + std::to_string(lock.l_pid) + " has a read lock already!";
            std::cout << err_message << std::endl;
            //close(fd);
            throw err_message;
            //exit(1);
        }

        fcntl(fd, F_SETLK, &savelock);
        //std::cout << savelock.l_type << std::endl;

        // 1. Read content:
        std::string file_content;
        char c;
        int read_flag = -1;

        while (read_flag != 0) {
            read_flag = read(fd, &c, 1);
            if (read_flag == -1) {
                std::string err_message = "Read error from file " + std::string(filename);
                std::cout << err_message << std::endl;
                // close(fd);
                throw err_message;
            }
            if (read_flag != 0)
                file_content += c;
        }
        //std::cout << file_content << std::endl;

        // 2. transform content:
        std::string new_content = transform_content(file_content);
        //std::cout << new_content << std::endl;

        // 3. Write new content:
        lseek(fd, 0, SEEK_SET);

        int file_size = new_content.size();
        if (file_size < file_content.size()) {
            file_size = file_content.size();
            new_content.append(file_size - new_content.length(), ' ');
        }
        write(fd, new_content.c_str(), new_content.size());

        close(fd);

    }
	

	std::string read_file_with_lock(const char* filename) {
		flock lock, savelock;
 		int fd;
   
  		fd = open(filename, O_RDONLY);
  		lock.l_type = F_RDLCK;
  		lock.l_start = 0;
  		lock.l_whence = SEEK_SET;
  		lock.l_len = 50;
  		savelock = lock;
  		fcntl(fd, F_GETLK, &lock);
  		if (lock.l_type == F_WRLCK)
  		{
			std::string err_message = "File is write-locked by process " + std::to_string(lock.l_pid) + "!";
                        std::cout << err_message << std::endl;
      			//close(fd);
			throw err_message;
      			//exit(1);
  		}
  		fcntl(fd, F_SETLK, &savelock);

		
		std::string file_content;
		char c;
		int read_flag = -1;
		
		while(read_flag != 0) {
			read_flag = read(fd, &c, 1);
			if (read_flag == -1) {
				std::string err_message = "Read error from file " + std::string(filename);
				std::cout << err_message << std::endl;
				// close(fd);
				throw err_message;
			}
            if (read_flag != 0) 
			    file_content += c;
		}

		close(fd);

		return file_content;
	}

}
