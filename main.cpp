#include <iostream>
#include <clocale>

#include "httplib.h"
#include "BookingManager.h"
#include "routes.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    
    cout << "=== Инициализация сервера ===" << endl;
    
    // Загрузка данных
    BookingManager manager;
    manager.loadData();
    
    // Создание сервера
    httplib::Server svr;
    
    // Регистрация маршрутов
    setupRoutes(svr, manager);
    
    // Запуск
    cout << "========================================" << endl;
    cout << "  Сервер запущен!" << endl;
    cout << "  http://localhost:8080" << endl;
    cout << "  Ctrl+C для остановки" << endl;
    cout << "========================================" << endl;
    
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}