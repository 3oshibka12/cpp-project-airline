#pragma once

#include <vector>
#include <string>
#include <mutex>

#include "Flight.h"
#include "Passenger.h"
#include "Booking.h"

using namespace std;

class BookingManager {
private:
    vector<Flight> flights;
    vector<Passenger> passengers;
    vector<Booking> bookings;
    
    mutex data_mtx;
    
    int getNextFlightId();
    int getNextPassengerId();
    int getNextBookingId();

    string generateSecretCode();
    
    Passenger* findPassengerByEmail(const string& email);
    
    Flight* findFlightById(int id);

public:
    static const string ADMIN_USERNAME;
    static const string ADMIN_PASSWORD;

    BookingManager() = default;
    
    void loadData();
    void saveData();
    
    vector<Flight> searchFlights(const string& from, const string& to, const string& date);
    
    bool bookFlight(int flightId, const string& name, const string& email, 
                    const string& phone, string& errorMsg);
    
    bool cancelBooking(int bookingId, const string& secretCode, string& errorMsg);
    
    vector<Booking> getBookingsByEmail(const string& email);
    
    vector<Booking> getAllBookings();
    
    vector<Flight> getAllFlights();
    
    Flight getFlightById(int id);
    
    Passenger getPassengerById(int id);
};