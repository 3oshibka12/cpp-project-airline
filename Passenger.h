#pragma once
#include <string>

class Passenger {
public:
    int id;
    std::string name;
    std::string email;
    std::string phone;

    Passenger(int id, std::string name, std::string email, std::string phone);
};