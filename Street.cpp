/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "helper_functions.h"

using namespace std;

Street::Street() {
    
}

Street::~Street() {
    
}

unsigned Street::getStreetId() {
    
    return streetId;
}
    
vector<unsigned> Street::getStreetSegments() {
    
    return streetSegments;
}

vector<unsigned> Street::getIntersectionIds() {
    
    return intersections;
} 

// Searches through the unordered map of the street to determine intersections 
// between the given street name and the street of the current Street object.
vector<unsigned> Street::getIntersectionFromStreetName(string streetName) {
    
    auto searchIter = intersectionStreets.find(streetName);
    // If there are no intersections found, returns an empty vector.
    if (searchIter == intersectionStreets.end()) {
        vector<unsigned> emptyVector;
        return emptyVector;
    }
    else {
        return searchIter -> second;
    }
}

void Street::setStreetId(const unsigned _streetId) {
    
    streetId = _streetId;
}


    
void Street::pushStreetSegment(const unsigned streetSegmentId) {
    
    streetSegments.push_back(streetSegmentId);
}
    
void Street::pushIntersection(const unsigned intersectionId) {
    
    intersections.push_back(intersectionId);
}

// Creates the unordered map to map streets with which this street intersects 
// and their corresponding intersection ID.
void Street::setIntersectionStreets() {
    
    vector<unsigned> streetsOfIntersection;
    unsigned currentStreetSegment;
    struct StreetSegmentInfo currentStreetSegmentInfo;
    unsigned currentStreet;
    string streetName;
    unordered_map<string, vector<unsigned>>::const_iterator searchIter;
    
    // Iterates through all of the intersections of the street.
    for (unsigned i = 0; i < intersections.size(); i++) {
        
        // Iterates through all of the street segments of the current 
        // intersection.
        for (unsigned j = 0; j < getIntersectionStreetSegmentCount(intersections[i]);
                j++) {
            currentStreetSegment = getIntersectionStreetSegment(intersections[i], j);
            currentStreetSegmentInfo = getStreetSegmentInfo(currentStreetSegment);
            currentStreet = currentStreetSegmentInfo.streetID;
            
            // Although the street is considered to intersect itself, it is much
            // faster to check a match on street ID than it is to create a pair
            // on the unordered map for the street. Therefore, the street itself
            // is ignored.
            if (currentStreet != streetId) {
                // Checks for and ignores duplicates; vector should only contain
                // each street once.
                if (!isDuplicate(streetsOfIntersection, currentStreet)) {
                    streetsOfIntersection.push_back(currentStreet);
                }
            }
        }
        // For every street found to intersect at this intersection, maps the
        // name of the street to this intersection ID.
        for (unsigned streetIndex = 0; streetIndex < streetsOfIntersection.size();
                streetIndex++) {
            streetName = getStreetName(streetsOfIntersection[streetIndex]);
            searchIter = intersectionStreets.find(streetName);
            // First checks if the street had intersected with this street
            // elsewhere. If it did, then the intersection ID is pushed back to
            // the mapping with the already existing pair.
            if (searchIter != intersectionStreets.end()) {
                intersectionStreets[streetName].push_back(intersections[i]);
            }
            // If it does not exist, then creates the pair in the unordered map.
            else {
                vector<unsigned> vectorToMap;
                vectorToMap.push_back(intersections[i]);
                intersectionStreets.emplace(streetName, vectorToMap);
            }
        }
        streetsOfIntersection.clear();
    }
}