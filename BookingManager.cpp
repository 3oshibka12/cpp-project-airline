#include "BookingManager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <random>
#include <ctime>

const string BookingManager::ADMIN_USERNAME = "admin";
const string BookingManager::ADMIN_PASSWORD = "password";


static vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


string BookingManager::generateSecretCode() {
    static mt19937 rng(time(nullptr));
    uniform_int_distribution<int> dist(100000, 999999);
    return to_string(dist(rng));
}

int BookingManager::getNextFlightId() {
    int maxId = 0;
    for (const auto& f : flights) {
        if (f.id > maxId) maxId = f.id;
    }
    return maxId + 1;
}

int BookingManager::getNextPassengerId() {
    int maxId = 0;
    for (const auto& p : passengers) {
        if (p.id > maxId) maxId = p.id;
    }
    return maxId + 1;
}

int BookingManager::getNextBookingId() {
    int maxId = 0;
    for (const auto& b : bookings) {
        if (b.id > maxId) maxId = b.id;
    }
    return maxId + 1;
}

Passenger* BookingManager::findPassengerByEmail(const string& email) {
    for (auto& p : passengers) {
        if (p.email == email) {
            return &p;
        }
    }
    return nullptr;
}

Flight* BookingManager::findFlightById(int id) {
    for (auto& f : flights) {
        if (f.id == id) {
            return &f;
        }
    }
    return nullptr;
}


void BookingManager::loadData() {
    lock_guard<mutex> lock(data_mtx);
    
    {
        ifstream file("data/flights.csv");
        if (file.is_open()) {
            string line;
            getline(file, line);
            
            while (getline(file, line)) {
                vector<string> data = split(line, ',');
                if (data.size() < 9) continue;
                
                flights.emplace_back(
                    stoi(data[0]), data[1], data[2], data[3],
                    data[4], data[5], stod(data[6]), stoi(data[7]), stoi(data[8])
                );
            }
            cout << "Загружено рейсов: " << flights.size() << endl;
        } else {
            cerr << "Предупреждение: файл flights.csv не найден" << endl;
        }
    }
    
    {
        ifstream file("data/passengers.csv");
        if (file.is_open()) {
            string line;
            getline(file, line);
            
            while (getline(file, line)) {
                vector<string> data = split(line, ',');
                if (data.size() < 4) continue;
                
                passengers.emplace_back(stoi(data[0]), data[1], data[2], data[3]);
            }
            cout << "Загружено пассажиров: " << passengers.size() << endl;
        } else {
            cerr << "Предупреждение: файл passengers.csv не найден" << endl;
        }
    }
    
    {
        ifstream file("data/bookings.csv");
        if (file.is_open()) {
            string line;
            getline(file, line);
            
            while (getline(file, line)) {
                vector<string> data = split(line, ',');
                if (data.size() < 5) continue;
                
                string code = (data.size() >= 6) ? data[5] : "";
                bookings.emplace_back(stoi(data[0]), stoi(data[1]), stoi(data[2]), data[3], data[4], code);
            }
            cout << "Загружено бронирований: " << bookings.size() << endl;
        } else {
            cerr << "Предупреждение: файл bookings.csv не найден" << endl;
        }
    }
}


void BookingManager::saveData() {
    lock_guard<mutex> lock(data_mtx);
    
    {
        ofstream file("data/flights.csv");
        if (file.is_open()) {
            file << "id,number,from,to,date,time,price,seats_total,seats_booked\n";
            for (const auto& f : flights) {
                file << f.id << "," << f.number << "," << f.from << "," << f.to << ","
                     << f.date << "," << f.time << "," << f.price << ","
                     << f.seats_total << "," << f.seats_booked << "\n";
            }
        }
    }
    
    {
        ofstream file("data/passengers.csv");
        if (file.is_open()) {
            file << "id,name,email,phone\n";
            for (const auto& p : passengers) {
                file << p.id << "," << p.name << "," << p.email << "," << p.phone << "\n";
            }
        }
    }
    
    {
        ofstream file("data/bookings.csv");
        if (file.is_open()) {
            file << "id,flight_id,passenger_id,seat_num,status,secret_code\n";
            for (const auto& b : bookings) {
                file << b.id << "," << b.flight_id << "," << b.passenger_id << ","
                    << b.seat_num << "," << b.status << "," << b.secret_code << "\n";
            }
        }
    }
    
    cout << "Данные сохранены" << endl;
}


vector<Flight> BookingManager::searchFlights(const string& from, const string& to, const string& date) {
    lock_guard<mutex> lock(data_mtx);
    
    vector<Flight> result;
    
    for (const auto& f : flights) {
        bool matchFrom = from.empty() || f.from == from;
        bool matchTo = to.empty() || f.to == to;
        bool matchDate = date.empty() || f.date == date;
        
        bool hasSeats = f.seats_booked < f.seats_total;
        
        if (matchFrom && matchTo && matchDate && hasSeats) {
            result.push_back(f);
        }
    }
    
    return result;
}


bool BookingManager::bookFlight(int flightId, const string& name, const string& email,
                                const string& phone, string& errorMsg) {
    lock_guard<mutex> lock(data_mtx);
    

    if (name.length() < 3) {
        errorMsg = "Введите полное имя (минимум 3 символа)";
        return false;
    }
    
    regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!regex_match(email, emailPattern)) {
        errorMsg = "Некорректный email адрес";
        return false;
    }
    
    regex phonePattern(R"([\+]?[78]?\d{10,11})");
    if (!regex_match(phone, phonePattern)) {
        errorMsg = "Некорректный номер телефона (формат: +79001234567)";
        return false;
    }


    Flight* flight = findFlightById(flightId);
    if (!flight) {
        errorMsg = "Рейс не найден";
        return false;
    }
    
    if (flight->seats_booked >= flight->seats_total) {
        errorMsg = "На этом рейсе нет свободных мест";
        return false;
    }
    
    Passenger* passenger = findPassengerByEmail(email);
    if (!passenger) {
        Passenger newPassenger(getNextPassengerId(), name, email, phone);
        passengers.push_back(newPassenger);
        passenger = &passengers.back();
        cout << "Создан новый пассажир: " << name << " (ID: " << passenger->id << ")" << endl;
    }
    
    int seatNumber = flight->seats_booked + 1;
    string seatNum = to_string(seatNumber) + "A";
    
    string code = generateSecretCode();
    Booking newBooking(getNextBookingId(), flightId, passenger->id, seatNum, "confirmed", code);
    bookings.push_back(newBooking);

    cout << "Создана бронь #" << newBooking.id << ", секретный код: " << code << endl;
    
    flight->seats_booked++;
    
    cout << "Создана бронь #" << newBooking.id << " на рейс " << flight->number 
         << ", место " << seatNum << ", код бронирования: " << code << endl;
    
    return true;
}


bool BookingManager::cancelBooking(int bookingId, const string& secretCode, string& errorMsg) {
    lock_guard<mutex> lock(data_mtx);
    
    for (auto& b : bookings) {
        if (b.id == bookingId) {
            if (b.status == "cancelled") {
                errorMsg = "Эта бронь уже отменена";
                return false;
            }
            
            // Проверка кода (пустой код = админ)
            if (!secretCode.empty() && b.secret_code != secretCode) {
                errorMsg = "Неверный секретный код";
                return false;
            }
            
            b.status = "cancelled";
            
            Flight* flight = findFlightById(b.flight_id);
            if (flight && flight->seats_booked > 0) {
                flight->seats_booked--;
            }
            
            cout << "Бронь #" << bookingId << " отменена" << endl;
            return true;
        }
    }
    
    errorMsg = "Бронь не найдена";
    return false;
}


vector<Booking> BookingManager::getBookingsByEmail(const string& email) {
    lock_guard<mutex> lock(data_mtx);
    
    vector<Booking> result;
    
    Passenger* passenger = findPassengerByEmail(email);
    if (!passenger) {
        return result;
    }
    
    for (const auto& b : bookings) {
        if (b.passenger_id == passenger->id) {
            result.push_back(b);
        }
    }
    
    return result;
}

vector<Booking> BookingManager::getAllBookings() {
    lock_guard<mutex> lock(data_mtx);
    return bookings;
}

vector<Flight> BookingManager::getAllFlights() {
    lock_guard<mutex> lock(data_mtx);
    return flights;
}

Flight BookingManager::getFlightById(int id) {
    lock_guard<mutex> lock(data_mtx);
    
    for (const auto& f : flights) {
        if (f.id == id) return f;
    }
    
    return Flight(-1, "", "", "", "", "", 0, 0, 0);
}

Passenger BookingManager::getPassengerById(int id) {
    lock_guard<mutex> lock(data_mtx);
    
    for (const auto& p : passengers) {
        if (p.id == id) return p;
    }
    
    return Passenger(-1, "", "", "");
}