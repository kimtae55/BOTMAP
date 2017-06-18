#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include "m1.h"
#include "helper_functions.h"
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "m2.h"
#include "m2_helper.h"
#include "graphics.h"
#include "LatLon.h"
#include "global_variables.h"

using namespace std;

void draw_map() {
    
    start_graphics();
    
    event_loop(act_on_mousebutton, nullptr, nullptr, draw_screen);
    
    close_graphics();

}

