#include "Booking.h"

Booking::Booking(int id, int flight_id, int passenger_id, std::string seat_num, std::string status)
    : id(id), flight_id(flight_id), passenger_id(passenger_id), seat_num(seat_num), status(status) {
}