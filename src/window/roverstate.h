#pragma once
#include <QString>

struct RoverState {
    double timestamp   = 0.0;
    double x           = 50.0;
    double y           = 50.0;
    double heading_deg = 0.0;
    double speed       = 0.0;
    double battery     = 100.0;
    int    uptime_sec  = 0;
    QString status     = "IDLE";
    int    error_code  = 0;
    QString event      = "";
};
