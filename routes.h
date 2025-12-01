#pragma once

#include "httplib.h"
#include "BookingManager.h"

// Регистрация всех маршрутов
void setupRoutes(httplib::Server& svr, BookingManager& manager);