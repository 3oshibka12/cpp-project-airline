#include "routes.h"
#include "utils.h"

#include <iostream>
#include <regex>

using namespace std;

void setupRoutes(httplib::Server& svr, BookingManager& manager) {
    
    // ==========================================
    // GET / — Главная страница
    // ==========================================
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(renderTemplate("index.html"), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // GET /flights — Все рейсы
    // ==========================================
    svr.Get("/flights", [&manager](const httplib::Request& req, httplib::Response& res) {
        auto flights = manager.getAllFlights();
        
        string rowsHtml;
        for (const auto& f : flights) {
            int available = f.seats_total - f.seats_booked;
            string availClass = (available > 0) ? "available" : "full";
            string availText = to_string(available) + " / " + to_string(f.seats_total);
            
            rowsHtml += "<tr>";
            rowsHtml += "<td><strong>" + f.number + "</strong></td>";
            rowsHtml += "<td>" + f.from + " → " + f.to + "</td>";
            rowsHtml += "<td>" + f.date + "</td>";
            rowsHtml += "<td>" + f.time + "</td>";
            rowsHtml += "<td>" + to_string((int)f.price) + " ₽</td>";
            rowsHtml += "<td><span class='" + availClass + "'>" + availText + "</span></td>";
            
            if (available > 0) {
                rowsHtml += "<td><a class='btn' href='/book/" + to_string(f.id) + "'>Забронировать</a></td>";
            } else {
                rowsHtml += "<td><span class='btn disabled'>Мест нет</span></td>";
            }
            rowsHtml += "</tr>";
        }
        
        map<string, string> data = {
            {"TOTAL", to_string(flights.size())},
            {"ROWS", rowsHtml}
        };
        res.set_content(renderTemplate("flights.html", data), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // POST /search — Поиск рейсов
    // ==========================================
    svr.Post("/search", [&manager](const httplib::Request& req, httplib::Response& res) {
        string from = req.get_param_value("from");
        string to = req.get_param_value("to");
        string date = req.get_param_value("date");
        
        auto flights = manager.searchFlights(from, to, date);
        
        string resultsHtml;
        if (flights.empty()) {
            resultsHtml = "<p class='no-results'>Рейсы не найдены. Попробуйте изменить параметры поиска.</p>";
        } else {
            for (const auto& f : flights) {
                int available = f.seats_total - f.seats_booked;
                resultsHtml += "<div class='flight'>";
                resultsHtml += "<div class='flight-info'>";
                resultsHtml += "<strong>" + f.number + "</strong>: " + f.from + " → " + f.to + "<br>";
                resultsHtml += "Дата: " + f.date + ", Время: " + f.time + "<br>";
                resultsHtml += "Свободных мест: " + to_string(available);
                resultsHtml += "</div>";
                resultsHtml += "<div class='flight-price'>" + to_string((int)f.price) + " ₽</div>";
                resultsHtml += "<a class='btn' href='/book/" + to_string(f.id) + "'>Забронировать</a>";
                resultsHtml += "</div>";
            }
        }
        
        map<string, string> data = {{"RESULTS", resultsHtml}};
        res.set_content(renderTemplate("search_results.html", data), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // GET /book/:id — Форма бронирования
    // ==========================================
    svr.Get(R"(/book/(\d+))", [&manager](const httplib::Request& req, httplib::Response& res) {
        int flightId = stoi(req.matches[1]);
        Flight flight = manager.getFlightById(flightId);
        
        if (flight.id == -1) {
            res.status = 404;
            res.set_content("<h1>Рейс не найден</h1><a href='/'>На главную</a>", "text/html; charset=utf-8");
            return;
        }
        
        map<string, string> data = {
            {"FLIGHT_ID", to_string(flight.id)},
            {"FLIGHT_NUMBER", flight.number},
            {"FLIGHT_FROM", flight.from},
            {"FLIGHT_TO", flight.to},
            {"FLIGHT_DATE", flight.date},
            {"FLIGHT_TIME", flight.time},
            {"FLIGHT_PRICE", to_string((int)flight.price)},
            {"ERROR", ""}
        };
        
        res.set_content(renderTemplate("booking_form.html", data), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // POST /book — Создание брони
    // ==========================================
    svr.Post("/book", [&manager](const httplib::Request& req, httplib::Response& res) {
        int flightId = stoi(req.get_param_value("flight_id"));
        string name = req.get_param_value("name");
        string email = req.get_param_value("email");
        string phone = req.get_param_value("phone");
        
        string error;
        bool success = manager.bookFlight(flightId, name, email, phone, error);
        
        if (success) {
            manager.saveData();
            
            Flight flight = manager.getFlightById(flightId);
            map<string, string> data = {
                {"MESSAGE", "Бронирование успешно!"},
                {"DETAILS", "Рейс " + flight.number + " (" + flight.from + " → " + flight.to + ")"},
                {"EMAIL", email}
            };
            res.set_content(renderTemplate("success.html", data), "text/html; charset=utf-8");
        } else {
            Flight flight = manager.getFlightById(flightId);
            map<string, string> data = {
                {"FLIGHT_ID", to_string(flight.id)},
                {"FLIGHT_NUMBER", flight.number},
                {"FLIGHT_FROM", flight.from},
                {"FLIGHT_TO", flight.to},
                {"FLIGHT_DATE", flight.date},
                {"FLIGHT_TIME", flight.time},
                {"FLIGHT_PRICE", to_string((int)flight.price)},
                {"ERROR", "<div class='error'>" + error + "</div>"}
            };
            res.set_content(renderTemplate("booking_form.html", data), "text/html; charset=utf-8");
        }
    });
    
    // ==========================================
    // GET /my-bookings — Мои брони
    // ==========================================
    svr.Get("/my-bookings", [&manager](const httplib::Request& req, httplib::Response& res) {
        string email = req.get_param_value("email");
        
        string bookingsHtml;
        if (email.empty()) {
            bookingsHtml = "<p>Введите email для поиска ваших бронирований.</p>";
        } else {
            auto bookings = manager.getBookingsByEmail(email);
            
            if (bookings.empty()) {
                bookingsHtml = "<p>Бронирования не найдены для " + email + "</p>";
            } else {
                for (const auto& b : bookings) {
                    Flight f = manager.getFlightById(b.flight_id);
                    string statusClass = (b.status == "confirmed") ? "status-confirmed" : "status-cancelled";
                    string statusText = (b.status == "confirmed") ? "Подтверждено" : "Отменено";
                    string bookingClass = (b.status == "cancelled") ? "booking cancelled" : "booking";
                    
                    bookingsHtml += "<div class='" + bookingClass + "'>";
                    bookingsHtml += "<div>";
                    bookingsHtml += "<strong>Бронь #" + to_string(b.id) + "</strong><br>";
                    bookingsHtml += "Рейс " + f.number + ": " + f.from + " → " + f.to + "<br>";
                    bookingsHtml += "Дата: " + f.date + ", Место: " + b.seat_num + "<br>";
                    bookingsHtml += "Статус: <span class='" + statusClass + "'>" + statusText + "</span>";
                    bookingsHtml += "</div>";
                    
                    if (b.status == "confirmed") {
                        bookingsHtml += "<form action='/cancel/" + to_string(b.id) + "' method='POST' style='margin:0'>";
                        bookingsHtml += "<input type='hidden' name='email' value='" + email + "'>";
                        bookingsHtml += "<button class='cancel' type='submit'>Отменить</button>";
                        bookingsHtml += "</form>";
                    }
                    bookingsHtml += "</div>";
                }
            }
        }
        
        map<string, string> data = {
            {"EMAIL", email},
            {"BOOKINGS", bookingsHtml}
        };
        res.set_content(renderTemplate("my_bookings.html", data), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // POST /cancel/:id — Отмена брони
    // ==========================================
    svr.Post(R"(/cancel/(\d+))", [&manager](const httplib::Request& req, httplib::Response& res) {
        int bookingId = stoi(req.matches[1]);
        string email = req.get_param_value("email");
        
        string error;
        manager.cancelBooking(bookingId, error);
        manager.saveData();
        
        res.set_redirect("/my-bookings?email=" + email);
    });
    
    // ==========================================
    // GET /admin — Админ-панель
    // ==========================================
    svr.Get("/admin", [&manager](const httplib::Request& req, httplib::Response& res) {
        auto bookings = manager.getAllBookings();
        
        string rowsHtml;
        for (const auto& b : bookings) {
            Flight f = manager.getFlightById(b.flight_id);
            Passenger p = manager.getPassengerById(b.passenger_id);
            string statusClass = (b.status == "confirmed") ? "status-confirmed" : "status-cancelled";
            string statusText = (b.status == "confirmed") ? "Подтверждено" : "Отменено";
            
            rowsHtml += "<tr>";
            rowsHtml += "<td>" + to_string(b.id) + "</td>";
            rowsHtml += "<td>" + f.number + " (" + f.from + "→" + f.to + ")</td>";
            rowsHtml += "<td>" + p.name + "<br><small>" + p.email + "</small></td>";
            rowsHtml += "<td>" + b.seat_num + "</td>";
            rowsHtml += "<td><span class='" + statusClass + "'>" + statusText + "</span></td>";
            rowsHtml += "</tr>";
        }
        
        map<string, string> data = {
            {"TOTAL", to_string(bookings.size())},
            {"ROWS", rowsHtml}
        };
        res.set_content(renderTemplate("admin.html", data), "text/html; charset=utf-8");
    });
    
    // ==========================================
    // Обработка 404
    // ==========================================
    svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        string html = R"(
            <!DOCTYPE html>
            <html lang="ru">
            <head>
                <meta charset="UTF-8">
                <title>404 - Не найдено</title>
                <style>
                    body { font-family: Arial, sans-serif; text-align: center; padding: 100px; }
                    h1 { font-size: 72px; color: #e74c3c; margin: 0; }
                    p { font-size: 24px; color: #7f8c8d; }
                    a { color: #3498db; }
                </style>
            </head>
            <body>
                <h1>404</h1>
                <p>Страница не найдена</p>
                <a href="/">← Вернуться на главную</a>
            </body>
            </html>
        )";
        res.set_content(html, "text/html; charset=utf-8");
    });
    
    cout << "Маршруты зарегистрированы" << endl;
}