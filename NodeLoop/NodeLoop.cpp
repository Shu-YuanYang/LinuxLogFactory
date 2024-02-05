// NodeLoop.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "nlohmann/json.hpp"
//#include <string>
#include "COrder.h"
#include <algorithm>





int main(int argc, char* argv[])
{
    //std::cout << "Hello World Main Thread!\n";
    std::string arg_str(argv[1]);
    std::replace(arg_str.begin(), arg_str.end(), '\'', '"');
    nlohmann::json Doc{ nlohmann::json::parse(arg_str)};
    
    LinuxLogFactory::SOrder sample_order{
        std::vector<std::string>{"s1", "s2", "s3"}, 
        std::chrono::system_clock::now(),
        "ssoiweiewef",
        1
    };

    nlohmann::json j = sample_order;
    std::cout << j << std::endl;
    LinuxLogFactory::SOrder confirm_order(j.template get<LinuxLogFactory::SOrder>());

    long long time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(confirm_order.deadline.time_since_epoch()).count();

    std::cout << Doc << std::endl;
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
