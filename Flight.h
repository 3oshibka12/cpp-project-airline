#pragma once
#include <iostream>
#include <string>

class Flight {
public:
    int id;
    std::string number;
    std::string from;
    std::string to;
    std::string date;
    std::string time;
    double price;
    int seats_total;
    int seats_booked;


    Flight(int id, std::string number, std::string from, std::string to,
        std::string date, std::string time, double price, int total, int booked);
};