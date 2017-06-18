/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DeliveryLocationTimes.h
 * Author: liangosc
 *
 * Created on April 7, 2017, 9:03 AM
 */

#include <unordered_map>

#include "m4.h"

#ifndef DELIVERYLOCATIONTIMES_H
#define DELIVERYLOCATIONTIMES_H

using namespace std;

class DeliveryLocationTimes {
public:
    DeliveryLocationTimes();
    virtual ~DeliveryLocationTimes();
    unordered_map<unsigned, unordered_map<unsigned, double>> getTimeMap();
    
    void clearMap();
    void setMap(const std::vector<DeliveryInfo>& deliveries, 
                const std::vector<unsigned>& depots, 
                const float turn_penalty);
    void modifiedDijkstra(const unsigned intersection, int numberOfLocations, const float turn_penalty);
private:

    unordered_map<unsigned, unordered_map<unsigned, double>> timeMap;
};

#endif /* DELIVERYLOCATIONTIMES_H */

