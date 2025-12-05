#pragma once
#include <string>

using namespace std;

class Booking {
public:
    int id;
    int flight_id;
    int passenger_id;
    string seat_num;
    string status;
    string secret_code; // Новое поле — секретный код для отмены

    Booking() = default;
    Booking(int id, int flight_id, int passenger_id, string seat_num, string status, string secret_code = "");
};