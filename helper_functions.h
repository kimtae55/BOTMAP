#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <vector>
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "LatLon.h"
#include "graphics.h"

#include "StreetsDatabaseAPI.h"
#include <iostream>
#include <string>
using namespace std;

struct FeatureInfo {
    FeatureType featureType;
    vector<t_point> featurePoints;    
    bool isClosed;
    float area;
    unsigned featureId;
};

void createStreetVector();

void createStreetSegmentMap();

void createIntersectionMap();

void createTravelTimeMap();

void createPoiTree();

void createIntersecTree();

void createDirectlyConnectedMap();

void deleteDataStructures();

bool isDuplicate(const std::vector<StreetIndex> &, StreetIndex);

void createAlreadyDrawn();

void createFeatureMap();

void load_osm(string);

void createOSMIDMap();

void createStreetTypeMap();

void createNameCollisionMap();

void createPoiMap();

std::string generate_osm_path(std::string);

bool areaComparison(struct FeatureInfo& fInfo1, struct FeatureInfo& fInfo2);

float compute_area(vector<t_point>& featurePoints, int numPoints);

#endif /* HELPER_FUNCTIONS_H */

