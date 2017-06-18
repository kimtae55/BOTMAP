/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <vector>
#include <string>
#include <algorithm>
#include <queue> 
#include <map>
#include <cmath>

#include "m1.h"
#include "helper_functions.h"
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "m2.h"
#include "m2_helper.h"
#include "graphics.h"
#include "LatLon.h"
#include "global_variables.h"
#include "m3.h"
#include "m3_classes.h"

extern bool positionSet;
extern t_point clickedPoint;
extern unsigned currentIntersection;

using namespace std;

unsigned lastIntersectionId;

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function
// can assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street
// segment, plus the given turn_penalty (in seconds) per turn implied by the path.
// A turn occurs when two consecutive street segments have different street IDs.
double compute_path_travel_time(const std::vector<unsigned>& path,
const double turn_penalty)
{
    double time = 0;
    // Iterate through the path and find total travel time for all street segments
    for (unsigned i = 0; i < path.size(); i++) 
    {
        time = time + find_street_segment_travel_time(path[i]);
        // If the next street segment is not the end of the path 
        if ((i+1) < path.size()) 
        {
            StreetSegmentInfo current = getStreetSegmentInfo(path[i]);
            StreetSegmentInfo next = getStreetSegmentInfo(path[i+1]);
            // Check if two consecutive street segments have the same street ID
            if (current.streetID != next.streetID)
                // If not plus the turn penalty given
                time = time + turn_penalty;
        }
    }
    // Return the total travel time in seconds including time taken to travel 
    // through all street segments and turn penalty time.
    return time;
}

// Returns a path (route) between the start intersection and the end
// intersection, if one exists. This routine should return the shortest path
// between the given intersections when the time penalty to turn (change
// street IDs) is given by turn_penalty (in seconds).
// If no path exists, this routine returns an empty (size == 0) vector.
// If more than one path exists, the path with the shortest travel time is
// returned. The path is returned as a vector of street segment ids; traversing
// these street segments, in the returned order, would take one from the start
// to the end intersection.
std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start,
const unsigned intersect_id_end, const double turn_penalty)
{
    
    priority_queue<IntersectionsAndTravelTimes, vector<IntersectionsAndTravelTimes>, ComparisonClass> evaluationQueue;
    unordered_map<unsigned, IntersectionsAndTravelTimes> evaluatedNodes;
    vector<bool> visited;
    
    // Set up visited flags.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        visited.push_back(false);
    }
    
    // Start by pushing the start intersection into the priority queue.
    IntersectionsAndTravelTimes start(intersect_id_start, 0, 0,0);
    evaluationQueue.push(start);
    IntersectionsAndTravelTimes currentInter = evaluationQueue.top();
    
    // Iterate through intersections until the shortest path has been found.
    while (currentInter.getIntersectionID() != intersect_id_end && !evaluationQueue.empty()) {
        // Check to prevent evaluating the same node more than once.
        if (!visited[currentInter.getIntersectionID()]) {
            vector<unsigned> connectedStreetSegs = find_intersection_street_segments(currentInter.getIntersectionID());
            // Evaluate the time cost of traveling to each node. Insert these nodes into the priority queue.
            for (unsigned i = 0; i < connectedStreetSegs.size(); i++) {
                struct StreetSegmentInfo currentInfo = getStreetSegmentInfo(connectedStreetSegs[i]);
                // A path would be invalid if going the wrong way on a one-way street.
                if (!currentInfo.oneWay || (currentInfo.oneWay && currentInfo.from == currentInter.getIntersectionID())) {
                    unsigned adjacentIntersection = (currentInfo.from == currentInter.getIntersectionID()) ? currentInfo.to : currentInfo.from;
                    // Check to prevent backtracking.
                    if (!visited[adjacentIntersection]) {
                        double currentTravelTime = find_street_segment_travel_time(connectedStreetSegs[i]);
                        // Adds the turn penalty if the streets are different.
                        if (currentInter.getPreviousStreetID() != currentInfo.streetID && currentInter.getIntersectionID() != intersect_id_start) {
                            currentTravelTime += turn_penalty;
                        }
                        // Create a new node to evaluate and pushes it on the evaluation queue.
                        unsigned segmentID = connectedStreetSegs[i];
                        IntersectionsAndTravelTimes nextIntersection(adjacentIntersection, currentInter.getTravelTime() + currentTravelTime, currentInfo.streetID, segmentID);
                        nextIntersection.setPreviousNode(currentInter.getIntersectionID());
                        evaluationQueue.push(nextIntersection);
                    }
                }
            }
            visited[currentInter.getIntersectionID()] = true;
            evaluatedNodes.emplace(currentInter.getIntersectionID(), currentInter);
        }
        evaluationQueue.pop();
        // Checks for empty queue so that no invalid reads occur.
        if (!evaluationQueue.empty()) {
            currentInter = evaluationQueue.top();
        }
    }
    vector<unsigned> pathVector;
    // If queue is empty, that means that no path was found.
    if(!evaluationQueue.empty()) {
        // Searches through nodes to find the determined shortest path.
        lastIntersectionId = currentInter.getIntersectionID();
        while(currentInter.getIntersectionID() != intersect_id_start) {
            pathVector.push_back(currentInter.getSegmentID());
            currentInter = evaluatedNodes[currentInter.getPreviousNode()];
        }
        reverse(pathVector.begin(), pathVector.end());
    }
    currentIntersection = lastIntersectionId;
    clickedPoint.x = longitude_to_cartesian(getIntersectionPosition(currentIntersection).lon(), currentMinMax.avgRadLat);
    clickedPoint.y = getIntersectionPosition(currentIntersection).lat();
    positionSet = true;
    return pathVector;
}

std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start,
const std::string point_of_interest_name, const double turn_penalty)
{
    vector<unsigned> closest_intersecs;
    vector<unsigned> no_path;
    vector<unsigned> path;

    // get a vector containing closest intersection IDs to all POI locations
    // based on the POI name string input 
    auto iter = POIname_InterID.find(point_of_interest_name);

    if (iter != POIname_InterID.end()) 
        closest_intersecs = iter->second;
        
    if (closest_intersecs.empty()) 
        return no_path;
    
    else{     
        path = find_closest_path_for_poi(intersect_id_start, closest_intersecs , turn_penalty);
        return path;
    }
}

vector<unsigned> find_closest_path_for_poi(const unsigned intersect_id_start, vector<unsigned> closest_intersecs, const double turn_penalty)
{
    
    priority_queue<IntersectionsAndTravelTimes, vector<IntersectionsAndTravelTimes>, ComparisonClass> evaluationQueue;
    unordered_map<unsigned, IntersectionsAndTravelTimes> evaluatedNodes;
    vector<bool> visited; 
    
    // Set up visited flags.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        visited.push_back(false);
    }
    IntersectionsAndTravelTimes start(intersect_id_start, 0, 0,0);
    evaluationQueue.push(start);
    IntersectionsAndTravelTimes currentInter = evaluationQueue.top();
    while ((find(closest_intersecs.begin(), closest_intersecs.end(), currentInter.getIntersectionID()) == closest_intersecs.end())
            && !evaluationQueue.empty()) {
        if (!visited[currentInter.getIntersectionID()]) {
            vector<unsigned> connectedStreetSegs = find_intersection_street_segments(currentInter.getIntersectionID());
            for (unsigned i = 0; i < connectedStreetSegs.size(); i++) {
                struct StreetSegmentInfo currentInfo = getStreetSegmentInfo(connectedStreetSegs[i]);
                if (!currentInfo.oneWay || (currentInfo.oneWay && currentInfo.from == currentInter.getIntersectionID())) {
                    unsigned adjacentIntersection = (currentInfo.from == currentInter.getIntersectionID()) ? currentInfo.to : currentInfo.from;
                    if (!visited[adjacentIntersection]) {
                        double currentTravelTime = find_street_segment_travel_time(connectedStreetSegs[i]);
                        if (currentInter.getPreviousStreetID() != currentInfo.streetID && currentInter.getIntersectionID() != intersect_id_start) {
                            currentTravelTime += turn_penalty;
                        }
                        unsigned segmentID = connectedStreetSegs[i];
                        IntersectionsAndTravelTimes nextIntersection(adjacentIntersection, currentInter.getTravelTime() + currentTravelTime, currentInfo.streetID, segmentID);
                        nextIntersection.setPreviousNode(currentInter.getIntersectionID());
                        evaluationQueue.push(nextIntersection);
                    }
                }
            }
            visited[currentInter.getIntersectionID()] = true;
            evaluatedNodes.emplace(currentInter.getIntersectionID(), currentInter);
        }
        evaluationQueue.pop();
        if (!evaluationQueue.empty()) {
            currentInter = evaluationQueue.top();
        }
    }
    if (evaluationQueue.empty()) {
        vector<unsigned> emptyVector;
        return emptyVector;
    }
    else {
        vector<unsigned> pathVector;
        lastIntersectionId = currentInter.getIntersectionID();

        while(currentInter.getIntersectionID() != intersect_id_start) {
            pathVector.push_back(currentInter.getSegmentID());
            currentInter = evaluatedNodes[currentInter.getPreviousNode()];
        }
        reverse(pathVector.begin(), pathVector.end());
        currentIntersection = lastIntersectionId;
        clickedPoint.x = longitude_to_cartesian(getIntersectionPosition(currentIntersection).lon(), currentMinMax.avgRadLat);
        clickedPoint.y = getIntersectionPosition(currentIntersection).lat();
        positionSet = true;
        return pathVector;
    }
}
