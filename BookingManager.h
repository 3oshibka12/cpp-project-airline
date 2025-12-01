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
    
    // Мьютекс для потокобезопасности (когда веб-сервер будет обрабатывать
    // несколько запросов одновременно)
    mutex data_mtx;
    
    // Вспомогательные методы для генерации ID
    int getNextFlightId();
    int getNextPassengerId();
    int getNextBookingId();
    
    // Поиск пассажира по email (возвращает указатель или nullptr)
    Passenger* findPassengerByEmail(const string& email);
    
    // Поиск рейса по ID
    Flight* findFlightById(int id);

public:
    BookingManager() = default;
    
    // === Загрузка и сохранение ===
    void loadData();
    void saveData();
    
    // === Основные операции ===
    
    // Поиск рейсов (from и to могут быть пустыми — тогда не фильтруем по ним)
    vector<Flight> searchFlights(const string& from, const string& to, const string& date);
    
    // Бронирование рейса
    // Возвращает true если успех, false если ошибка (текст ошибки в errorMsg)
    bool bookFlight(int flightId, const string& name, const string& email, 
                    const string& phone, string& errorMsg);
    
    // Отмена брони
    bool cancelBooking(int bookingId, string& errorMsg);
    
    // Получить брони по email пассажира
    vector<Booking> getBookingsByEmail(const string& email);
    
    // Получить все брони (для админки)
    vector<Booking> getAllBookings();
    
    // Получить все рейсы
    vector<Flight> getAllFlights();
    
    // Получить рейс по ID (для формы бронирования)
    Flight getFlightById(int id);
    
    // Получить пассажира по ID (для отображения информации)
    Passenger getPassengerById(int id);
};