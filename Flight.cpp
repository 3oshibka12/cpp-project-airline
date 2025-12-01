#include "Flight.h"

Flight::Flight(int id, std::string number, std::string from, std::string to,
    std::string date, std::string time, double price, int total, int booked)
    : id(id), number(number), from(from), to(to),
    date(date), time(time), price(price), seats_total(total), seats_booked(booked) {
}