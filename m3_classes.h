/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3_classes.h
 * Author: liangosc
 *
 * Created on March 21, 2017, 3:32 PM
 */

#ifndef M3_CLASSES_H
#define M3_CLASSES_H

class IntersectionsAndTravelTimes {
public:
    IntersectionsAndTravelTimes(unsigned _id, double _travelTime, unsigned _previousStreetID, unsigned _seg_id) {
        intersectionID = _id;
        travelTime = _travelTime;
        previousStreetID = _previousStreetID;
        seg_ID = _seg_id;
        
    }
    IntersectionsAndTravelTimes() {
        
    }
    unsigned getIntersectionID() {return intersectionID;}
    double getTravelTime() {return travelTime;}
    unsigned getPreviousNode() {return previousNode;}
    unsigned getPreviousStreetID() {return previousStreetID;}
    void setPreviousNode(const unsigned _previousNode) {previousNode = _previousNode;}
    unsigned getSegmentID() {return seg_ID;}
    
private:
    unsigned intersectionID;
    double travelTime;
    unsigned previousNode;
    unsigned previousStreetID;
    unsigned seg_ID;
};

class ComparisonClass {
public:
    bool operator() (IntersectionsAndTravelTimes & lhs, IntersectionsAndTravelTimes & rhs) {
        return lhs.getTravelTime() > rhs.getTravelTime();
    }
};

#endif /* M3_CLASSES_H */

