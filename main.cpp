#include <iostream>
#include <clocale>

#include "httplib.h"
#include "BookingManager.h"
#include "routes.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    
    cout << "Сервер создается" << endl;
    
    BookingManager manager;
    manager.loadData();
    
    httplib::Server svr;
    
    setupRoutes(svr, manager);
    
    cout << "Сервер запустился" << endl;
    cout << "Тык-тык: http://localhost:8080" << endl;
    
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}