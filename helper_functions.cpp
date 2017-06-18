#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>

#include "helper_functions.h"
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "m2_helper.h"
#include "m3.h"
#include "LatLon.h"
#include "OSMDatabaseAPI.h"
#include "global_variables.h"

using namespace std;

// Function to create a vector of Street object pointers, which is a class with
// useful information about the street, such as its segments and intersections.


void createStreetVector() {
    
    StreetIndex streetOfStreetSegment;
    vector<StreetIndex> uniqueStreets;
    
    // Iterate through all streets to create their corresponding Street objects.
    // The street objects will be indexed in the vector by their street ID.
    for (unsigned i = 0; i < getNumberOfStreets(); i++) {
        Street currentStreet;
        currentStreet.setStreetId(i);
        streetVector.push_back(currentStreet);
    }

    // Iterate through all street segments to determine the street to which it 
    // belongs, and pushing it on to that street's vector of street segments.
    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
        
        streetVector[getStreetSegmentInfo(i).streetID].pushStreetSegment(i);
    }
    
    // Iterates through all the intersections. For each intersection, checks 
    // the street segments to see which streets the intersection connects, and
    // pushes the intersection onto the vector of intersections that belong to
    // that street.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        for (unsigned j = 0; j < getIntersectionStreetSegmentCount(i); j++) {
            StreetSegmentIndex currentStreetSegmentId = getIntersectionStreetSegment(i, j);
            streetOfStreetSegment = getStreetSegmentInfo(currentStreetSegmentId).streetID;
            // Each street should only contain the intersection once.
            if (!isDuplicate(uniqueStreets, streetOfStreetSegment)) {
                uniqueStreets.push_back(streetOfStreetSegment);
                streetVector[streetOfStreetSegment].pushIntersection(i);
            }
        }
        uniqueStreets.clear();
    }
    
    // Calls another function to create the unordered map to map a street's 
    // intersections and the names of the streets with which it intersects.
    for (unsigned i = 0; i < streetVector.size(); i++) {
        streetVector[i].setIntersectionStreets();
    }
}

// Creates global unordered map that maps street name to a vector of street IDs 
// with that name.
void createStreetSegmentMap() {
    
    string currentStreetName;
    vector<unsigned>* listOfStreetIds;
    unordered_map<string, vector<unsigned>*>::const_iterator searchIter;
    
    for (unsigned i = 0; i < getNumberOfStreets(); i++) {
        currentStreetName = getStreetName(i);
        searchIter = streetNameToId.find(currentStreetName);
        // Condition to check if string was successfully found.
        if(searchIter == streetNameToId.end()) {
            // If not found, creates the new pair and inserts into unordered
            // map.
            listOfStreetIds = new vector<unsigned>;
            listOfStreetIds -> push_back(i);
            streetNameToId.emplace(currentStreetName, listOfStreetIds);
        }
        else {
            // If found, adds the street id to the vector.
            searchIter -> second -> push_back(i);
        }
    }
}

// Function to create the global vector to map an intersection to its connecting
// street segments.
void createIntersectionMap() {
    
    vector<unsigned> street_seg_id;
    
    // Iterates through all intersections.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        int num_ss = int (getIntersectionStreetSegmentCount(i));

        // Iterates through all the street segments of the current intersection.
        for (int index = 0; index < num_ss; index++) {
            street_seg_id.push_back(getIntersectionStreetSegment(i, index));
        }
        intersectionsToStreetSegments.push_back(street_seg_id);
        street_seg_id.clear();
    }
}

// Function to create the global vector to map street segments to the time it 
// takes to travel through it at its speed limit.
void createTravelTimeMap() {
    /*Iterate through all street segments and store the corresponding travel
      time for each segment in the mapping vector segmentsToTravelTime  */ 
    double time;
    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {

        // Travel time is calculated as distance (in metres) divided by the
        // speed (in kilometres per hour, converted to metres per second using
        // a factor of 3.6). 
        time = (find_street_segment_length(i) / 
                getStreetSegmentInfo(i).speedLimit) * 3.6;   
        segmentsToTravelTime.push_back(time);       
    }
}

// Creates an r-tree to store points of interest data.
void createPoiTree() {   
    /*Iterate through all poi to store data points for position into r-tree
      using insert member function */
    unsigned poi_size = getNumberOfPointsOfInterest();
    for (unsigned i = 0; i < poi_size; i++)
    {
        LatLon poi_pos = getPointOfInterestPosition(i);
        point poi_posc = point(poi_pos.lat(), poi_pos.lon());
        poitree.insert(std::make_pair(poi_posc,i));
    }
}
       
// Creates an r-tree to store intersection data.
void createIntersecTree()
{
    // Iterate through all intersections to store data points for position into
    // r-tree using insert member function 
    unsigned intersec_size = getNumberOfIntersections();
    for (unsigned i = 0; i < intersec_size; i++)
    {
        LatLon intersec_pos = getIntersectionPosition(i);
        point intersec_posc = point(intersec_pos.lat(), intersec_pos.lon());
        intersectree.insert(std::make_pair(intersec_posc,i));
    }
}

// Creates a vector of unordered maps. The vector is indexed by intersections,
// and the unordered map determines the intersections to which it is connected.
void createDirectlyConnectedMap() {
    
    unsigned numOfStreetSegments;
    unordered_map<unsigned, bool>* mapOfConnectedIntersections;
    unsigned currentStreetSegment;
    struct StreetSegmentInfo currentStreetSegmentInfo;
    
    // Iterate through all intersections to create the structure.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        mapOfConnectedIntersections = new unordered_map<unsigned, bool>;
        numOfStreetSegments = getIntersectionStreetSegmentCount(i);
        
        // Iterate through all of the street segments that connect to that 
        // intersection.
        for (unsigned j = 0; j < numOfStreetSegments; j++) {
            currentStreetSegment = getIntersectionStreetSegment(i, j);
            currentStreetSegmentInfo = getStreetSegmentInfo(currentStreetSegment);
            if (currentStreetSegmentInfo.from == i) {
                mapOfConnectedIntersections -> emplace(currentStreetSegmentInfo.to,
                        true);
            }
            // If oneWay were true, then the street would be a one way street
            // going the wrong way, so it is not connected in that direction.
            // It is assumed that if the current intersection is not "from,"
            // then it must be "to."
            else if (currentStreetSegmentInfo.oneWay == false) {
                mapOfConnectedIntersections -> emplace(currentStreetSegmentInfo.from,
                        true);
            }
        }
        directlyConnectedMap.push_back(mapOfConnectedIntersections);
    }
}

//Delete any data structures used and frees all allocated memory.
void deleteDataStructures() {
    
    streetVector.clear();
    
    for (auto& i: streetNameToId) {
        delete i.second;
    }
    streetNameToId.clear();
    
    intersectionsToStreetSegments.clear();
    
    segmentsToTravelTime.clear(); 
    
    poitree.clear();
    
    intersectree.clear();
    
    for (unsigned i = 0; i < directlyConnectedMap.size(); i++) {
        delete directlyConnectedMap[i];
    }
    directlyConnectedMap.clear();
    
    alreadyDrawn.clear();
    
    featureMap.clear();
    
    OSMIDMap.clear();
    
    streetTypeMap.clear();
    
    nameCollision.clear();

    POIname_InterID.clear();
}

// Checks if the given street index is a duplicate of an already existing 
// element in the vector. Returns true if it is a duplicate.
bool isDuplicate(const vector<StreetIndex> & uniqueElements, StreetIndex    
        streetToCompare) {
    
    for (unsigned i = 0; i < uniqueElements.size(); i++) {
        if (streetToCompare == uniqueElements[i]) {
            return true;
        }
    }
    return false;
}

void createAlreadyDrawn() {
    
    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
        alreadyDrawn.push_back(false);
    }
}

void createFeatureMap() {
  
    for (unsigned i = 0; i < getNumberOfFeatures(); i++) {
        struct FeatureInfo featureInformation;
        featureInformation.featureId = i;
        LatLon start = getFeaturePoint(i, 0);
        LatLon end = getFeaturePoint(i, getFeaturePointCount(i)-1);
        featureInformation.isClosed = ((start.lat() == end.lat()) && (start.lon() == end.lon()));    
        
        featureInformation.featureType = getFeatureType(i);
        
        unsigned numPoints = getFeaturePointCount(i);
        if (numPoints > 0) {
        }

        for (unsigned j = 0; j < numPoints; j++) {
            //get the coordinate of the current feature point in LatLon type
            // featurePoints is a vector of t_point which holds x and y
            LatLon feature_point = getFeaturePoint(i, j);
            t_point newPoint;
            newPoint.x = longitude_to_cartesian(feature_point.lon(), currentMinMax.avgRadLat);
            newPoint.y = feature_point.lat();
            featureInformation.featurePoints.push_back(newPoint);
        }
        if (featureInformation.isClosed) {
            featureInformation.area = compute_area(featureInformation.featurePoints, getFeaturePointCount(i));
        }
        else {
            featureInformation.area = 0;
        }
        featureMap.push_back(featureInformation); 
    }
    std::sort(featureMap.begin(), featureMap.end(), areaComparison);
}

// Load osm data base for according map
void load_osm(string osm_path) {
    
    loadOSMDatabaseBIN(osm_path);
}

// Create map from OSMID to OSMWay
void createOSMIDMap() {
    
    for (unsigned i = 0; i < getNumberOfWays(); i++) {
        const OSMWay* currentWay = getWayByIndex(i);
        OSMIDMap.emplace(currentWay -> id(), currentWay);
    }
}

//Create map from OSMway to tag which contains the street type 
void createStreetTypeMap() {
    
    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
        struct StreetSegmentInfo currentStreetSeg = getStreetSegmentInfo(i);
        const OSMWay* currentOSMWay = OSMIDMap[currentStreetSeg.wayOSMID];
        for (unsigned j = 0; j < getTagCount(currentOSMWay); j++) {
            string key, value;
            tie(key, value) = getTagPair (currentOSMWay, j);
            if (key == "highway") {
                streetTypeMap.push_back(value);
            }
        }
    }
}

// if too slow, try using R-Tree 
void createNameCollisionMap() {
//
//    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
//        
//        vector<t_bound_box> box_bounds;
//        struct StreetSegmentInfo current = getStreetSegmentInfo(i);
//        
//        if (current.curvePointCount < 5) {
//
//            LatLon startPosition = getIntersectionPosition(current.from);
//            LatLon endPosition = getIntersectionPosition(current.to);
//            t_point bottomleft, topright;
//            bottomleft.x = longitude_to_cartesian(startPosition.lon(), currentMinMax.avgRadLat);
//            bottomleft.y = startPosition.lat();
//            topright.x = longitude_to_cartesian(endPosition.lon(), currentMinMax.avgRadLat);
//            topright.y = endPosition.lat();
//
//            // box from one end to the other end 
//            t_bound_box init(bottomleft, topright);   
//            box_bounds.push_back(init);
//              // this is the method i wanna use D: to compare all sides of each text box :S too slow
//            bool doesIntersect = false;
//            for (unsigned j = 0; j < box_bounds.size(); j = j + 2) {
//                if (i != j) {
//                    if (box_bounds[i].intersects(box_bounds[j].left(), box_bounds[j].top()) ||
//                        box_bounds[i].intersects(box_bounds[j].left(), box_bounds[j].bottom()) ||
//                        box_bounds[i].intersects(box_bounds[j].right(), box_bounds[j].top()) ||
//                        box_bounds[i].intersects(box_bounds[j].right(), box_bounds[j].bottom())
//                       ) {
//                        doesIntersect = true;
//                    }
//                    else {
//                        doesIntersect = false;
//                    }
//                }
//                else {
//                    doesIntersect = false;
//                }
//            }
//            nameCollision.push_back(doesIntersect);
//        }
//        else {
//            nameCollision.push_back(true);
//        }
        
//        t_point center;
//
//        // positioning
//        if (current.curvePointCount != 0 && current.curvePointCount < 4) {
//                   // get average LatLon of the curvepoints
//            double sumLat = 0, sumLon = 0;
//            for (unsigned j = 0; j < current.curvePointCount; j++) {
//                LatLon currentPoint = getStreetSegmentCurvePoint(i, j);
//                sumLon = sumLon + currentPoint.lon();
//                sumLat = sumLat + currentPoint.lat();
//            }
//            LatLon startie = getIntersectionPosition(current.from);
//            LatLon endie = getIntersectionPosition(current.to);
//            sumLon = sumLon + startie.lon() + endie.lon();
//            sumLat = sumLat + startie.lat() + endie.lat();
//
//            double averageLon = sumLon / double((current.curvePointCount+2));
//            double averageLat = sumLat / double((current.curvePointCount+2));
//            center.x = longitude_to_cartesian(averageLon, currentMinMax.avgRadLat);
//            center.y = averageLat;
//
//            if (init.intersects(center.x, center.y)) {
//                nameCollision.push_back(true);
//            }
//            else {
//                nameCollision.push_back(false);
//            }  
//        }
//        else if (current.curvePointCount != 0 && current.curvePointCount >= 4) {
//            nameCollision.push_back(false);
//        }
//        else {
//            IntersectionIndex start = current.from;
//            IntersectionIndex end = current.to;
//            // LatLon of a street seg
//            LatLon startPosition = getIntersectionPosition(start); 
//            // LatLon of the endpoint of that street seg
//            LatLon endPosition = getIntersectionPosition(end);
//
//            center.y = (startPosition.lat() + endPosition.lat())*0.5;
//            center.x = (startPosition.lon()*cos(currentMinMax.avgRadLat) 
//                       + endPosition.lon()*cos(currentMinMax.avgRadLat))*0.5;
//            
//            if (init.intersects(center.x, center.y)) {
//                nameCollision.push_back(true);
//            }
//            else {
//                nameCollision.push_back(false);
//            }            
//        }    
//    }
}

string generate_osm_path(string map_path) {
    // Changes the map path so that the osm can be loaded.
    size_t start = map_path.find("streets");
    string osm_path = map_path.replace(start, 7, "osm");
    return osm_path;
}

bool areaComparison(struct FeatureInfo& fInfo1, struct FeatureInfo& fInfo2) {
    
    // Function used to sort the areas of the features in std::sort. Sorts from
    // greatest to smallest.
    
    return (fInfo1.area > fInfo2.area);
}

float compute_area(vector<t_point>& featurePoints, int numPoints) {
    
    // Compute area using the shoelace formula, which is used to sort the array
    // of features.
    float area = 0.0;
    int j = numPoints - 1;
    for (int i = 0; i < numPoints; i++) {
        area += (featurePoints[i].x + featurePoints[j].x) * (featurePoints[j].y - 
                featurePoints[i].y);
        j = i;
    }
    return abs(area/2.0);
}

void createPoiMap() 
{   
    string name;
    unsigned closestId;
    
    for (unsigned i = 0 ; i < getNumberOfPointsOfInterest(); i++) {
        name = getPointOfInterestName(i);
        LatLon position = getPointOfInterestPosition(i);
        
        closestId = find_closest_intersection(position);
        
        if (POIname_InterID.find(name) != POIname_InterID.end()) {
            POIname_InterID[name].push_back(closestId);
        }
        else {
            vector<unsigned> ids;
            ids.push_back(closestId);
            POIname_InterID[name] = ids;
        }
    }
}
