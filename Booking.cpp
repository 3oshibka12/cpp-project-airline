#include "Booking.h"

Booking::Booking(int id, int flight_id, int passenger_id, string seat_num, string status, string secret_code)
    : id(id), flight_id(flight_id), passenger_id(passenger_id), 
      seat_num(seat_num), status(status), secret_code(secret_code) {
}