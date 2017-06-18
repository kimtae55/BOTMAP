/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <unordered_map>
#include <vector>
#include <chrono>
#include <algorithm>
#include <list>

#include "DeliveryLocationTimes.h"
#include "m4.h"
#include "m4_helper.h"
#include "m3.h"

DeliveryLocationTimes deliveryMap;

using namespace std;
#define TIME_LIMIT 30

// for optimization,
// should start algorithm at different startpoints
// and use two opt for each path until time runs out
// to compare in the two opt, use the pre compute time that oscar made

double best_time = 0;
double even_betterest = 10000000;

vector<unsigned> optimize(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty) {
    
        deliveryMap.clearMap();
    deliveryMap.setMap(deliveries, depots, turn_penalty);
    auto startTime = chrono::high_resolution_clock::now();
    bool timeOut = false;
    unsigned startDepotIndex, endDepotIndex;
    vector<unsigned> initalDeliveryOrder;
    vector<unsigned> deliveryOrder;
    vector<unsigned> evenBetterDeliveryOrder;
    
    // chooses random depot indexes for multistart 
    startDepotIndex = rand() % depots.size();
    endDepotIndex = rand() % depots.size();
    
    while(!timeOut) {
        initalDeliveryOrder = findGreedyPath(depots[startDepotIndex], depots[endDepotIndex], deliveries);
        // returns the order with best time for this choice
        deliveryOrder = startTwoOpt(initalDeliveryOrder, deliveries, depots);
        
        if (best_time < even_betterest) { 
            even_betterest = best_time;
            evenBetterDeliveryOrder = deliveryOrder;
        }
        
        startDepotIndex = rand() % depots.size();
        endDepotIndex = rand() % depots.size();
        
        // also want to start at random depots.... will the time be enough?
        auto currentTime = chrono::high_resolution_clock::now();
        auto wallClock = chrono::duration_cast<chrono::duration<double>>
                                        (currentTime - startTime);
        if (wallClock.count() > 0.9 * TIME_LIMIT) {
                timeOut = true;
        }
    }

    vector<unsigned> bestPath = reconstruct_path(evenBetterDeliveryOrder, turn_penalty);

    return bestPath;
}

vector<unsigned> startTwoOpt(vector<unsigned> deliveryOrder, const std::vector<DeliveryInfo>& deliveries,  const std::vector<unsigned>& depots) {
    vector<unsigned> newDeliveryOrder;
    vector<unsigned> bestDeliveryOrder;
    double new_time;
    unsigned size = deliveryOrder.size();
    int sameResult = 0;
    bestDeliveryOrder = deliveryOrder;
    best_time = calculateTime(deliveryOrder);

    // Exit if result doesn't change after awhile
        // startDepot and endDepot shouldn't matter for these swaps
        for (unsigned i = 1; i < size - 2; i++) {
            for (unsigned j = i + 1; j < size - 1; j++) {
                newDeliveryOrder = twoOpt(i, j,deliveryOrder, depots);
                //  valid order checking
                if (check_legal(newDeliveryOrder, deliveries, depots)) {
                    new_time = calculateTime(newDeliveryOrder);
                }

                if (new_time < best_time) {
                    sameResult = 0;
                    bestDeliveryOrder = newDeliveryOrder;
                    best_time = new_time;
                }
            }
        }
        
    // use this vector to trace the shortest connecting segments
    return bestDeliveryOrder;
}

vector<unsigned> twoOpt(int i, int j, vector<unsigned> deliveryOrder,  const std::vector<unsigned>& depots) {
    int size = deliveryOrder.size();

    // change this so that it fits whatever Oscar or Benji's code
    vector<unsigned> newDeliveryOrder;    
    
    // take route[1] to route[i-1] and add them in order
    for ( int index= 1; index<= i - 1; ++index)
    {
            newDeliveryOrder.push_back(deliveryOrder[index]);
    }

    // take route[i] to route[j] and add them in reverse order
    int dec = 0;
    for ( int index= i; index<= j; ++index)
    {
            newDeliveryOrder.push_back(deliveryOrder[j - dec]);
            dec++;
    }
    
    // take route[j+1] to end and add them in order
    for ( int index= j + 1; index< size-1; ++index)
    {
            newDeliveryOrder.push_back(deliveryOrder[index]);
    }
    
    
    // for choosing start depot
    double cost = 100000000;
    double newCost;
    unsigned index = 0;
    for (unsigned c = 0; c < depots.size(); c++) {
        newCost = deliveryMap.getTimeMap()[depots[c]][newDeliveryOrder[0]];
        if (newCost < cost) {
            cost = newCost;
            index = c;
        }
    }
    newDeliveryOrder.insert(newDeliveryOrder.begin(), depots[index]);
    
    // for end depot
    cost = 100000000;
    for (unsigned c = 0; c < depots.size(); c++) {
        newCost = deliveryMap.getTimeMap()[newDeliveryOrder[newDeliveryOrder.size()-1]][depots[c]];
        if (newCost < cost) {
            cost = newCost;
            index = c;
        }
    }
    newDeliveryOrder.insert(newDeliveryOrder.end(), depots[index]);
    
    return newDeliveryOrder;
}

double calculateTime(vector<unsigned> deliveryOrder) {
    auto timeMap = deliveryMap.getTimeMap();
    double cost = 0;
    for (unsigned i = 0; i < deliveryOrder.size()-1; i++) {
        cost += timeMap[deliveryOrder[i]][deliveryOrder[i+1]];
    }
    return cost;
}

// assumes that deliveryOrder is valid, returns street segments
// this is the final step for the travelling salesman problem
vector<unsigned> reconstruct_path(vector<unsigned> deliveryOrder, const float turn_penalty) {
    vector<unsigned> path;
    vector<unsigned> bestPath;
    unsigned start, end;
    for (unsigned i = 0; i < deliveryOrder.size()-1; i++) {
        
        start = deliveryOrder[i];
        end = deliveryOrder[i+1];
        path = find_path_between_intersections(start, end, turn_penalty);
        for (unsigned j = 0; j < path.size(); j++) {
            bestPath.push_back(path[j]);
        }
    }
    return bestPath;
}


bool check_legal(vector<unsigned> path, const std::vector<DeliveryInfo>& deliveries,  const std::vector<unsigned>& depots)
{
   unsigned initial_depot = path[0]; 
   unsigned final_depot = path.back();

    if ((std::find (depots.begin(), depots.end(), initial_depot) == depots.end()) ||
        (std::find (depots.begin(), depots.end(), final_depot) == depots.end()))
             return false;

    for (unsigned i = 0; i < deliveries.size(); i++) 
    { 
        vector<unsigned>:: iterator it; 
        unsigned pickup_id = deliveries[i].pickUp;
        unsigned dropoff_id =  deliveries[i].dropOff;
        it = find(path.begin(), path.end(), pickup_id);
        int pos1 = distance(path.begin(),it);
        it = find(path.begin(), path.end(), dropoff_id);
        int pos2 = distance(path.begin(),it);
        if (pos2 < pos1)
           return false;
    }
   return true;
}




vector<unsigned> findGreedyPath(unsigned startDepot, unsigned endDepot, const vector<DeliveryInfo>& deliveries) {
    
    vector<unsigned> greedyPath;
    list<DeliveryInfo> legalDropOffs;
    list<DeliveryInfo> illegalDropOffs;
    for (unsigned i = 0; i < deliveries.size(); i++) {
        illegalDropOffs.push_back(deliveries[i]);
    }
    greedyPath.push_back(startDepot);
    while (!(legalDropOffs.empty() && illegalDropOffs.empty())) {
        unordered_map<unsigned, double> locationTimeMap = deliveryMap.getTimeMap().find(greedyPath[greedyPath.size() - 1]) -> second;
        list<DeliveryInfo>::iterator nearestLegalLocationIter;
        bool inIllegalDropOffs = true;
        double shortestTime = RAND_MAX;
        for (auto iter = illegalDropOffs.begin(); iter != illegalDropOffs.end(); iter++) {
            if (locationTimeMap[iter -> pickUp] < shortestTime) {
                nearestLegalLocationIter = iter;
                shortestTime = locationTimeMap[iter -> pickUp];
            }
        }
        for (auto iter = legalDropOffs.begin(); iter != legalDropOffs.end(); iter++) {
            if (locationTimeMap[iter -> dropOff] < shortestTime) {
                nearestLegalLocationIter = iter;
                shortestTime = locationTimeMap[iter -> dropOff];
                inIllegalDropOffs = false;
            }
        }
//        for (int i = 0; i < illegalDropOffs.size(); i++) {
//            if (locationTimeMap[illegalDropOffs[i].pickUp] < shortestTime) {
//                nearestLegalLocationIndex = i;
//                shortestTime = locationTimeMap[illegalDropOffs[i].pickUp];
//            }
//        }
//        for (int i = 0; i < legalDropOffs.size(); i++) {
//            if (locationTimeMap[legalDropOffs[i].dropOff] < shortestTime) {
//                nearestLegalLocationIndex = i;
//                shortestTime = locationTimeMap[legalDropOffs[i].dropOff];
//                inIllegalDropOffs = false;
//            }
//        }
        unsigned nextLocation;
        if (inIllegalDropOffs) {
            nextLocation = nearestLegalLocationIter -> pickUp;
            legalDropOffs.push_back(*nearestLegalLocationIter);
            illegalDropOffs.erase(nearestLegalLocationIter);
        }
        else {
            nextLocation = nearestLegalLocationIter -> dropOff;
            legalDropOffs.erase(nearestLegalLocationIter);
        }
        if (locationTimeMap[nextLocation] == 0) {
            if (nextLocation != greedyPath[greedyPath.size() - 1]) {
                // If the value is 0, it is either because it is the same location
                // as the previous one, or a path could not be found. In this case,
                // there is no valid path connecting the intersections, and the
                // function returns an empty vector.
                vector<unsigned> emptyVector;
                return emptyVector;
            }
        }
        else {
            greedyPath.push_back(nextLocation);
        }
    }
    greedyPath.push_back(endDepot);
    
    return greedyPath;
}
