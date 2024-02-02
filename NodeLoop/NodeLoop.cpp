// NodeLoop.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "nlohmann/json.hpp";



void scheduler_run(int ID) {
    long count = 0;
    long end = 10000000000000000;
    while (true) {
        
        ++count;
        if (count == end) {
            count = 0;
            std::cout << "Check!" << std::endl;
        }
    }
}


int main(int argc, char* argv[])
{
    //std::cout << "Hello World Main Thread!\n";

    nlohmann::json Doc{ nlohmann::json::parse(argv[1])};
    
    std::cout << Doc[0] << std::endl;
    //std::thread scheduler_th(scheduler_run, 5);
    //scheduler_th.join();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
