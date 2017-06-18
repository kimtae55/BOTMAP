#ifndef M3_H
#define M3_H

#include <vector>
#include <string>
#include <LatLon.h>

using namespace std;

//
//// Returns the time required to travel along the path specified, in seconds.
//// The path is given as a vector of street segment ids, and this function
//// can assume the vector either forms a legal path or has size == 0.
//// The travel time is the sum of the length/speed-limit of each street
//// segment, plus the given turn_penalty (in seconds) per turn implied by the path.
//// A turn occurs when two consecutive street segments have different street IDs.
double compute_path_travel_time(const vector<unsigned>& path,
const double turn_penalty);

vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start,
const unsigned intersect_id_end, const double turn_penalty);

vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start,
const std::string point_of_interest_name, const double turn_penalty);

double find_distance_squared ( LatLon point1 , LatLon point2 );


std::vector<unsigned> find_closest_path_for_poi(const unsigned intersect_id_start,
vector<unsigned> closest_intersecs, const double turn_penalty);

#endif /* M3_H */

