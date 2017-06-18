#ifndef M2_HELPER_H
#define M2_HELPER_H

#include "m2.h"
#include "m3.h"
#include "graphics.h"
#include "LatLon.h"

#include <vector>
#include <string>

using namespace std;

typedef struct minMax {
    double maxLat;
    double minLat;
    double avgRadLat;
    double maxLon;
    double minLon;
    t_bound_box maxBox;
} MinMaxLatLon;

void draw_screen();

void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info);
void when_not_clicked();


float longitude_to_cartesian(float longitude, float average_latitude);
float cartesian_to_longitude(float x, float average_latitude);
float get_average_latitude();
void highlightAndDisplayIntersection(float x_inter, float y_inter);


void find_button_function(void (*drawscreen) (void));
void switch_map_button(void(*drawscreen) (void));
void ShowPOI_button(void (*drawscreen) (void));
void ShowPOIname_button(void (*drawscreen) (void));
void ShowMinor_button(void (*drawscreen) (void));
void bookmark_button(void (*drawscreen) (void));
void help_button(void (*drawscreen) (void));
void find_path_button(void (*drawscreen) (void));
void draw_helper();
void start_graphics();

void draw_polygon(unsigned feature_id, unsigned index, t_color colour);
void draw_line(unsigned feature_id, unsigned index, color_types colour, int linewidth);
void draw_features(int zoomLevel);
void draw_streets(int lineWidth);
void draw_point_of_interest(int zoomLevel);
void set_min_max();
int find_zoom_level();
int find_line_width(int zoomLevel);
bool setStreetProperties(int zoomLevel, unsigned streetSegment);
void draw_street_name(int zoomLevel); 
float streetNameAngle(unsigned streetSegIndex);
t_point getCenterForStreetSeg(unsigned streetSegIndex);
void find_poi_path_button(void (*drawscreen) (void));
void determine_directions(vector<unsigned> path, unsigned finalIntersection);
double round_nearest_ten_metres(double);
string kilometres_or_metres(double distance);
double cross_product_of_two_streets(struct StreetSegmentInfo inStreet, struct StreetSegmentInfo outStreet);

void draw_start_ping();


void draw_path(vector<unsigned> path, int zoomLevel, unsigned finalIntersection);

struct intersection_data {
    LatLon position;
    string name;
};

#endif /* M2_HELPER_H */

