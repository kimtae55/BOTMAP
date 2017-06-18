
/*
 * In m1.cpp, all the API functions required for milestone 1 has been created.
 * They have been modified to work with the unordered map and vectors that 
 * we have created for more flexible mapping and faster access. Functionality 
 * and Performance tests have been passed. 
 */

#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "m1.h"
#include "m2_helper.h"
#include "helper_functions.h"
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

using namespace std; 

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef std::pair<point, unsigned> info;

bgi::rtree< info, bgi::linear<16>> poitree;
bgi::rtree< info, bgi::linear<16>> intersectree;

unordered_map<string, vector<unsigned>*> streetNameToId;

vector<vector<unsigned>> intersectionsToStreetSegments;

vector<double> segmentsToTravelTime;

vector<unordered_map<unsigned, bool>*> directlyConnectedMap;

vector<Street> streetVector;

vector<bool> alreadyDrawn;

vector<struct FeatureInfo> featureMap;

unordered_map<OSMID, const OSMWay*> OSMIDMap;

vector<std::string> streetTypeMap;

vector<LatLon> listOfBookmarks;

MinMaxLatLon currentMinMax;

vector<struct intersection_data> IntersectionData;

vector<bool> nameCollision;

unordered_map<string, vector<unsigned>> POIname_InterID;


bool load_map(string map_name/*map_path*/) {
    bool successfulLoad;
    successfulLoad = loadStreetsDatabaseBIN(map_name);
    
    // If the map loads, then create all map related data structures
    if (successfulLoad) {
        load_osm(generate_osm_path(map_name));
        createStreetVector();
        createStreetSegmentMap();
        createIntersectionMap();
        createTravelTimeMap();
        createPoiTree();
        createIntersecTree();
        createDirectlyConnectedMap();
        createAlreadyDrawn();
        set_min_max();
        createFeatureMap();
        createOSMIDMap();
        createStreetTypeMap();
        createNameCollisionMap();
        createPoiMap();
    }
    
    return successfulLoad;
}

void close_map() {
    //Clean-up your map related data structures here
    
    deleteDataStructures();
    closeStreetDatabase();
    closeOSMDatabase();
}

// Returns street id(s) for the given street name
// If no street with this name exists , returns a 0 - length vector .
vector<unsigned> find_street_ids_from_name(string street_name) {
    vector<unsigned> street_ids;
    unordered_map<string, vector<unsigned>*>::const_iterator i = 
            streetNameToId.find(street_name);
    
    // Street was found (doesn't reach the end of the unordered map)).
    if (i != streetNameToId.end()) {
        street_ids = *(i->second);
    }
    
    // If street was not found, then the vector is empty.

    return street_ids;  
}

// Returns the street segments for the given intersection
vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {

    if (intersection_id < getNumberOfIntersections()) {
        return intersectionsToStreetSegments[intersection_id];
    }
    else {
        // Return empty vector
        vector<unsigned> emptyVector;
        return emptyVector;
    }
}

// Returns the street names at the given intersection
// (includes duplicate street names in returned vector)
vector<string> find_intersection_street_names(unsigned intersection_id) {
    vector<string> streetNames;

    // check if intersection_id is in-range
    if (intersection_id < getNumberOfIntersections()) {
        // valid
        // gets the segment ids at the intersection
        vector<unsigned> street_seg_id = find_intersection_street_segments(intersection_id);
        for (auto seg_id = street_seg_id.begin(); seg_id != street_seg_id.end(); seg_id++) {
            // check for duplicity since there could be two way streets --> two seg
            // in this case, since street_seg_id already includes the unique
            // segIDs, it automatically takes the duplicity into account
            StreetSegmentInfo streetInfoo = getStreetSegmentInfo(*seg_id);
            streetNames.push_back(getStreetName(streetInfoo.streetID));
        }
    }
    return streetNames;
}

// Function that checks whether two intersections are connected 
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    
    // Corner case where an intersection is considered connected to itself.
    if (intersection_id1 == intersection_id2) {
        return true;
    }
    // Corner case to check if intersections are out of range.
    if (intersection_id1 >= getNumberOfIntersections() || intersection_id2
            >= getNumberOfIntersections()) {
        return false;
    }
    unordered_map<unsigned, bool>* currentIntersectionConnections = 
            directlyConnectedMap[intersection_id1];
    
    // Searches for the intersection on the unordered map to see if it is 
    // directly connected.
    if (currentIntersectionConnections -> find(intersection_id2) == 
            currentIntersectionConnections -> end()) {
        return false;
    }
    else {
        return true;
    }
}

vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    
    vector<unsigned> adjacentIntersections;
    unordered_map<unsigned, bool>* mapOfAdjacentIntersections;
    
    // Corner case to check if intersection is out of range.
    if (intersection_id < getNumberOfIntersections()) {
    // Accesses the directlyConnectedMap to determine the intersections to
    // which it is connected.
        mapOfAdjacentIntersections = directlyConnectedMap[intersection_id];

        // Iterates through all of the intersections to which it is connected and
        // pushes the intersection IDs onto the vector.
        for (auto& i: *mapOfAdjacentIntersections) {
            adjacentIntersections.push_back(i.first);
        }
    }
    return adjacentIntersections;
}

vector<unsigned> find_street_street_segments(unsigned street_id) {
    
    // returns the vector of street segments from the global vector of Street
    // objects. Also checks for out of range.
    if (street_id < getNumberOfStreetSegments()) {
        return streetVector[street_id].getStreetSegments();
    }
    else {
        vector<unsigned> emptyVector;
        return emptyVector;
    }
    
}

vector<unsigned> find_all_street_intersections(unsigned street_id) {

    
    // returns the vector of intersection IDs from the global vector of
    // Street objects. Also checks for out of range.
    if (street_id < getNumberOfStreets()) {
        return streetVector[street_id].getIntersectionIds();
    }
    else {
        vector<unsigned> emptyVector;
        return emptyVector;
    }
}

vector<unsigned> find_intersection_ids_from_street_names(string street_name1, string street_name2) {

    vector<unsigned> allIntersections;
    vector<unsigned> streetIds1;
    vector<unsigned> currentIntersectionIds;
    Street currentStreet;
    
    // Finds all street IDs of the first street name to check its intersections.
    streetIds1 = find_street_ids_from_name(street_name1);
    for(unsigned i = 0; i < streetIds1.size(); i++) {
        
        currentStreet = streetVector[streetIds1[i]];
        // Accesses the unordered map of the street to find its intersections.
        currentIntersectionIds = currentStreet.getIntersectionFromStreetName(street_name2);
        for (unsigned j = 0; j < currentIntersectionIds.size(); j++) {
            allIntersections.push_back(currentIntersectionIds[j]);
        }
    }
    return allIntersections;
}

// Returns the distance between two coordinates in meters
double find_distance_between_two_points ( LatLon point1 , LatLon point2 ) 
{
    // using projection to convert from lat and lon to Cartesian coordinates
    double latavg, distance, x1, x2, y1, y2;
    y1 = point1.lat() * DEG_TO_RAD;
    y2 = point2.lat() * DEG_TO_RAD;
    latavg = (y1 + y2) / 2.0;
    x1 = (point1.lon() * DEG_TO_RAD) * cos(latavg);
    x2 = (point2.lon() * DEG_TO_RAD) * cos(latavg);
    distance = EARTH_RADIUS_IN_METERS
	       * sqrt((y2 - y1) * (y2 - y1)+ (x2 - x1)* (x2 - x1));  
    
    return distance;     
}

// Returns the length of the given street segment in meters
double find_street_segment_length ( unsigned street_segment_id ) 
{
    double seglength = 0;
    StreetSegmentInfo ssi = getStreetSegmentInfo (street_segment_id);
    LatLon to = getIntersectionPosition(ssi.to);
    LatLon from = getIntersectionPosition(ssi.from);
	
    /* This loop calculates segment length excluding the part from the last 
       curve point to the end of the street segment. Not executed if there is    
       no curve point*/

    for (unsigned i = 0; i < ssi.curvePointCount; i++)    
    {
        to = getStreetSegmentCurvePoint(street_segment_id,i);
        seglength += find_distance_between_two_points(to, from);
        from = getStreetSegmentCurvePoint(street_segment_id,i);
    }   
     
    /*Calculate the distance between the last curve point and the end of the 
      street segment. Also used for street segment with no curve point*/
    to = getIntersectionPosition(ssi.to);
    seglength += find_distance_between_two_points(to, from);
    
    return seglength;
}

// Returns the length of the specified street in meters
double find_street_length ( unsigned street_id )
{  
    vector<unsigned> totalseg;
    double street_length = 0;
    
    totalseg = find_street_street_segments (street_id);
    // Iterate through all street segments for a given street id and find the 
    // length of each segment
    for (unsigned i = 0; i < totalseg.size(); i++)
    {
        street_length += find_street_segment_length (totalseg[i]);
    }
     
    return street_length;
}

/* Returns the travel time to drive a street segment in seconds ( 
   time = distance /speed_limit )*/
double find_street_segment_travel_time ( unsigned street_segment_id )
{
    //Use the helper function that maps street segment id to travel time
    return (segmentsToTravelTime[street_segment_id]);
}

// Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest ( LatLon my_position ) 
{ 
    point poi_position = point(my_position.lat(), my_position.lon());
    std::vector<info> poi_info;
    //Use r-tree nearest neighbours query to returns 400 values closest to 
    // poi_position, and store the returned result to vector poi_info
    poitree.query(bgi::nearest(poi_position,400), std::back_inserter(poi_info));
    
    unsigned size = getNumberOfPointsOfInterest();   
    unsigned poi_id = 0;
    
    double distance; 
    LatLon poi_0 = getPointOfInterestPosition(poi_info[0].second);
    // compute the distance from given position to poi indexed 0
    double distance_new = find_distance_between_two_points (my_position, poi_0);
    
    // Iterate through 400 closest poi or total number of poi if the size is 
    // less than 400, compare the distance from my_position to each poi to find 
    // the closest poi id
    for (unsigned i = 0; i < 400 && i < size; i++)
    {
        distance = find_distance_between_two_points(my_position, 
                 getPointOfInterestPosition(poi_info[i].second));
        if (distance <distance_new)
        {
            distance_new = distance;
            poi_id = i;
        }
    }
    return poi_info[poi_id].second;
}


// Returns the the nearest intersection to the given position
unsigned find_closest_intersection (LatLon my_position) {
    point intersec_position = point(my_position.lat(), my_position.lon());
    std::vector<info> intersec_info;
    //Use r-tree nearest neighbours query to returns 400 values closest to 
    //intersec_position, and store the returned result to vector intersec_info
    intersectree.query(bgi::nearest(intersec_position,400), 
                       std::back_inserter(intersec_info));
    
    unsigned intersec_id = 0;
    
    double distance; 
    LatLon intersec_0 = getIntersectionPosition(intersec_info[0].second);
    // compute the distance from given position to the intersection indexed 0
    double distance_new = find_distance_between_two_points (my_position, intersec_0);
    
    // Iterate through 400 closest intersections or total number of intersections 
    // if the size is less than 400, compare the distance from my_position to 
    // each intersection to find the closest intersection id
    for (unsigned i = 0; i < 400 && i < getNumberOfIntersections(); i++)
    {
        distance = find_distance_between_two_points(my_position, 
                 getIntersectionPosition(intersec_info[i].second));
        if (distance < distance_new)
        {
            distance_new = distance;
            intersec_id = i;
        }
    }
    return intersec_info[intersec_id].second;
}

