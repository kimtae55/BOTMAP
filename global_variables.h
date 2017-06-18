/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <vector>
#include <unordered_map>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "m2_helper.h"

#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

using namespace std;

// Global variables used in the program.

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef std::pair<point, unsigned> info;

extern unordered_map<string, vector<unsigned>*> streetNameToId;

extern vector<vector<unsigned>> intersectionsToStreetSegments;

extern vector<double> segmentsToTravelTime;

extern vector<unordered_map<unsigned, bool>*> directlyConnectedMap;

extern vector<Street> streetVector;

extern vector<bool> alreadyDrawn;

extern vector<struct FeatureInfo> featureMap;

extern unordered_map<OSMID, const OSMWay*> OSMIDMap;

extern vector<string> streetTypeMap;

extern bgi::rtree< info, bgi::linear<16>> poitree;

extern bgi::rtree< info, bgi::linear<16>> intersectree;

extern vector<unsigned> listOfBookmarks;

extern MinMaxLatLon currentMinMax;

extern vector<struct intersection_data> IntersectionData;

extern vector<bool> nameCollision;

extern unordered_map<string, vector<unsigned>> POIname_InterID;

extern unsigned lastIntersectionId;


#endif /* GLOBAL_VARIABLES_H */

