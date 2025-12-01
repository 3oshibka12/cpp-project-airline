#pragma once
#include <string>

class Booking {
public:
    int id;
    int flight_id;
    int passenger_id;
    std::string seat_num;
    std::string status;

    Booking(int id, int flight_id, int passenger_id, std::string seat_num, std::string status);
};