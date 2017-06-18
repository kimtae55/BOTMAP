/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DeliveryLocationTimes.cpp
 * Author: liangosc
 * 
 * Created on April 7, 2017, 9:03 AM
 */

#include <unordered_map>
#include <queue>

#include "m4.h"
#include "DeliveryLocationTimes.h"
#include "m3_classes.h"
#include "StreetsDatabaseAPI.h"
#include "m1.h"

DeliveryLocationTimes::DeliveryLocationTimes() {
}

DeliveryLocationTimes::~DeliveryLocationTimes() {
}

unordered_map<unsigned, unordered_map<unsigned, double>> DeliveryLocationTimes::getTimeMap() {
    
    return timeMap;
}

void DeliveryLocationTimes::clearMap() {
    
    timeMap.clear();
}
    
void DeliveryLocationTimes::setMap(const std::vector<DeliveryInfo>& deliveries, 
                                   const std::vector<unsigned>& depots, 
                                   const float turn_penalty) {
    
    unordered_map<unsigned, double> travelTimesForDeliveries;
    int numberOfLocations = 0;
    
    for (unsigned i = 0; i < deliveries.size(); i++) {
        if (travelTimesForDeliveries.find(deliveries[i].dropOff) == travelTimesForDeliveries.end()) {
            travelTimesForDeliveries.emplace(deliveries[i].dropOff, 0);
            numberOfLocations++;
        }
        if (travelTimesForDeliveries.find(deliveries[i].pickUp) == travelTimesForDeliveries.end()) {
            travelTimesForDeliveries.emplace(deliveries[i].pickUp, 0);
            numberOfLocations++;
        }
    }
    for (unsigned i = 0; i < depots.size(); i++) {
        if (travelTimesForDeliveries.find(depots[i]) == travelTimesForDeliveries.end()) {
            travelTimesForDeliveries.emplace(depots[i], 0);
            numberOfLocations++;
        }
    }
    for (unsigned i = 0; i < deliveries.size(); i++) {
        if (timeMap.find(deliveries[i].dropOff) == timeMap.end()) {
            timeMap.emplace(deliveries[i].dropOff, travelTimesForDeliveries);
            modifiedDijkstra(deliveries[i].dropOff, numberOfLocations, turn_penalty);
        }
        if (timeMap.find(deliveries[i].pickUp) == timeMap.end()) {
            timeMap.emplace(deliveries[i].pickUp, travelTimesForDeliveries);
            modifiedDijkstra(deliveries[i].pickUp, numberOfLocations, turn_penalty);
        }
    }
    for (unsigned i = 0; i < depots.size(); i++) {
        if (timeMap.find(depots[i]) == timeMap.end()) {
            timeMap.emplace(depots[i], travelTimesForDeliveries);
            modifiedDijkstra(depots[i], numberOfLocations, turn_penalty);
        }
    }
}

void DeliveryLocationTimes::modifiedDijkstra(const unsigned intersection, int numberOfLocations, const float turn_penalty) {
    priority_queue<IntersectionsAndTravelTimes, vector<IntersectionsAndTravelTimes>, ComparisonClass> evaluationQueue;
    unordered_map<unsigned, IntersectionsAndTravelTimes> evaluatedNodes;
    vector<bool> visited;
    
    // Set up visited flags.
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        visited.push_back(false);
    }
    
    // Start by pushing the start intersection into the priority queue.
    IntersectionsAndTravelTimes start(intersection, 0, 0,0);
    evaluationQueue.push(start);
    IntersectionsAndTravelTimes currentInter = evaluationQueue.top();
    
    // Iterate through intersections until path to all locations has been found.
    while (numberOfLocations != 0 && !evaluationQueue.empty()) {
        // Check to prevent evaluating the same node more than once.
        if (!visited[currentInter.getIntersectionID()]) {
            vector<unsigned> connectedStreetSegs = find_intersection_street_segments(currentInter.getIntersectionID());
            // Evaluate the time cost of traveling to each node. Insert these nodes into the priority queue.
//#pragma omp parallel for
            for (unsigned i = 0; i < connectedStreetSegs.size(); i++) {
                struct StreetSegmentInfo currentInfo = getStreetSegmentInfo(connectedStreetSegs[i]);
                // A path would be invalid if going the wrong way on a one-way street.
                if (!currentInfo.oneWay || (currentInfo.oneWay && currentInfo.from == currentInter.getIntersectionID())) {
                    unsigned adjacentIntersection = (currentInfo.from == currentInter.getIntersectionID()) ? currentInfo.to : currentInfo.from;
                    // Check to prevent backtracking.
                    if (!visited[adjacentIntersection]) {
                        double currentTravelTime = find_street_segment_travel_time(connectedStreetSegs[i]);
                        // Adds the turn penalty if the streets are different.
                        if (currentInter.getPreviousStreetID() != currentInfo.streetID && currentInter.getIntersectionID() != intersection) {
                            currentTravelTime += turn_penalty;
                        }
                        // Create a new node to evaluate and pushes it on the evaluation queue.
                        unsigned segmentID = connectedStreetSegs[i];
                        IntersectionsAndTravelTimes nextIntersection(adjacentIntersection, currentInter.getTravelTime() + currentTravelTime, currentInfo.streetID, segmentID);
                        nextIntersection.setPreviousNode(currentInter.getIntersectionID());
//#pragma omp critical
                        evaluationQueue.push(nextIntersection);
                    }
                }
            }
            auto deliveryStartAndEnd = timeMap.find(intersection) -> second.find(currentInter.getIntersectionID());
            if(deliveryStartAndEnd != timeMap.find(intersection) -> second.end()) {
                deliveryStartAndEnd -> second = currentInter.getTravelTime();
                numberOfLocations--;
//                cout << "path from " << intersection << " to " << currentInter.getIntersectionID() << " found." << endl;
//                cout << "time taken is " << currentInter.getTravelTime() << endl;
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
    //cout << "done search for " << intersection << endl;
}
