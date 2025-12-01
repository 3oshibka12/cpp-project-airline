#include "BookingManager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

// === Вспомогательная функция split (такая же как в main) ===
static vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// === Генерация ID ===
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

// === Поиск по email ===
Passenger* BookingManager::findPassengerByEmail(const string& email) {
    for (auto& p : passengers) {
        if (p.email == email) {
            return &p;
        }
    }
    return nullptr;
}

// === Поиск рейса по ID ===
Flight* BookingManager::findFlightById(int id) {
    for (auto& f : flights) {
        if (f.id == id) {
            return &f;
        }
    }
    return nullptr;
}

// ============================================================
// ЗАГРУЗКА ДАННЫХ
// ============================================================
void BookingManager::loadData() {
    lock_guard<mutex> lock(data_mtx);
    
    // --- Загрузка рейсов ---
    {
        ifstream file("data/flights.csv");
        if (file.is_open()) {
            string line;
            getline(file, line); // пропускаем заголовок
            
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
    
    // --- Загрузка пассажиров ---
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
    
    // --- Загрузка броней ---
    {
        ifstream file("data/bookings.csv");
        if (file.is_open()) {
            string line;
            getline(file, line);
            
            while (getline(file, line)) {
                vector<string> data = split(line, ',');
                if (data.size() < 5) continue;
                
                bookings.emplace_back(
                    stoi(data[0]), stoi(data[1]), stoi(data[2]), data[3], data[4]
                );
            }
            cout << "Загружено бронирований: " << bookings.size() << endl;
        } else {
            cerr << "Предупреждение: файл bookings.csv не найден" << endl;
        }
    }
}

// ============================================================
// СОХРАНЕНИЕ ДАННЫХ
// ============================================================
void BookingManager::saveData() {
    lock_guard<mutex> lock(data_mtx);
    
    // --- Сохранение рейсов ---
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
    
    // --- Сохранение пассажиров ---
    {
        ofstream file("data/passengers.csv");
        if (file.is_open()) {
            file << "id,name,email,phone\n";
            for (const auto& p : passengers) {
                file << p.id << "," << p.name << "," << p.email << "," << p.phone << "\n";
            }
        }
    }
    
    // --- Сохранение броней ---
    {
        ofstream file("data/bookings.csv");
        if (file.is_open()) {
            file << "id,flight_id,passenger_id,seat_num,status\n";
            for (const auto& b : bookings) {
                file << b.id << "," << b.flight_id << "," << b.passenger_id << ","
                     << b.seat_num << "," << b.status << "\n";
            }
        }
    }
    
    cout << "Данные сохранены" << endl;
}

// ============================================================
// ПОИСК РЕЙСОВ
// ============================================================
vector<Flight> BookingManager::searchFlights(const string& from, const string& to, const string& date) {
    lock_guard<mutex> lock(data_mtx);
    
    vector<Flight> result;
    
    for (const auto& f : flights) {
        // Проверяем каждое условие (если параметр пустой — пропускаем проверку)
        bool matchFrom = from.empty() || f.from == from;
        bool matchTo = to.empty() || f.to == to;
        bool matchDate = date.empty() || f.date == date;
        
        // Проверяем, есть ли свободные места
        bool hasSeats = f.seats_booked < f.seats_total;
        
        if (matchFrom && matchTo && matchDate && hasSeats) {
            result.push_back(f);
        }
    }
    
    return result;
}

// ============================================================
// БРОНИРОВАНИЕ
// ============================================================
bool BookingManager::bookFlight(int flightId, const string& name, const string& email,
                                 const string& phone, string& errorMsg) {
    lock_guard<mutex> lock(data_mtx);
    

    // === ВАЛИДАЦИЯ ===
    
    // Проверка имени (минимум 2 слова)
    if (name.length() < 3) {
        errorMsg = "Введите полное имя (минимум 3 символа)";
        return false;
    }
    
    // Проверка email через регулярное выражение
    regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!regex_match(email, emailPattern)) {
        errorMsg = "Некорректный email адрес";
        return false;
    }
    
    // Проверка телефона (должен начинаться с + или 8, минимум 10 цифр)
    regex phonePattern(R"([\+]?[78]?\d{10,11})");
    if (!regex_match(phone, phonePattern)) {
        errorMsg = "Некорректный номер телефона (формат: +79001234567)";
        return false;
    }


    // 1. Ищем рейс
    Flight* flight = findFlightById(flightId);
    if (!flight) {
        errorMsg = "Рейс не найден";
        return false;
    }
    
    // 2. Проверяем наличие мест
    if (flight->seats_booked >= flight->seats_total) {
        errorMsg = "На этом рейсе нет свободных мест";
        return false;
    }
    
    // 3. Ищем или создаём пассажира
    Passenger* passenger = findPassengerByEmail(email);
    if (!passenger) {
        // Создаём нового пассажира
        Passenger newPassenger(getNextPassengerId(), name, email, phone);
        passengers.push_back(newPassenger);
        passenger = &passengers.back();
        cout << "Создан новый пассажир: " << name << " (ID: " << passenger->id << ")" << endl;
    }
    
    // 4. Генерируем номер места (простая логика: следующее по счёту)
    int seatNumber = flight->seats_booked + 1;
    string seatNum = to_string(seatNumber) + "A"; // Например: "51A"
    
    // 5. Создаём бронь
    Booking newBooking(getNextBookingId(), flightId, passenger->id, seatNum, "confirmed");
    bookings.push_back(newBooking);
    
    // 6. Увеличиваем счётчик занятых мест
    flight->seats_booked++;
    
    cout << "Создана бронь #" << newBooking.id << " на рейс " << flight->number 
         << ", место " << seatNum << endl;
    
    return true;
}

// ============================================================
// ОТМЕНА БРОНИ
// ============================================================
bool BookingManager::cancelBooking(int bookingId, string& errorMsg) {
    lock_guard<mutex> lock(data_mtx);
    
    // Ищем бронь
    for (auto& b : bookings) {
        if (b.id == bookingId) {
            // Проверяем, не отменена ли уже
            if (b.status == "cancelled") {
                errorMsg = "Эта бронь уже отменена";
                return false;
            }
            
            // Меняем статус
            b.status = "cancelled";
            
            // Освобождаем место на рейсе
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

// ============================================================
// ПОЛУЧЕНИЕ БРОНЕЙ ПО EMAIL
// ============================================================
vector<Booking> BookingManager::getBookingsByEmail(const string& email) {
    lock_guard<mutex> lock(data_mtx);
    
    vector<Booking> result;
    
    // Сначала находим пассажира
    Passenger* passenger = findPassengerByEmail(email);
    if (!passenger) {
        return result; // Пустой вектор, пассажир не найден
    }
    
    // Собираем все его брони
    for (const auto& b : bookings) {
        if (b.passenger_id == passenger->id) {
            result.push_back(b);
        }
    }
    
    return result;
}

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ ГЕТТЕРЫ
// ============================================================
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
    
    // Возвращаем "пустой" рейс с id = -1 как признак ошибки
    return Flight(-1, "", "", "", "", "", 0, 0, 0);
}

Passenger BookingManager::getPassengerById(int id) {
    lock_guard<mutex> lock(data_mtx);
    
    for (const auto& p : passengers) {
        if (p.id == id) return p;
    }
    
    return Passenger(-1, "", "", "");
}