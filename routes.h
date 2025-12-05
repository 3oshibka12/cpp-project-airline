#pragma once

#include "httplib.h"
#include "BookingManager.h"

void setupRoutes(httplib::Server& svr, BookingManager& manager);