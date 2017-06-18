#include <unordered_map>
#include <vector>
#include <string>

#ifndef STREET_H
#define STREET_H

class Street {
    
public:
    
    Street();
    ~Street();
    
    // Accessor functions
    unsigned getStreetId();
    std::vector<unsigned> getStreetSegments();
    std::vector<unsigned> getIntersectionIds();
    std::vector<unsigned> getIntersectionFromStreetName(std::string streetName);
    
    // Mutator functions
    void setStreetId(const unsigned _streetId);
    void pushStreetSegment(const unsigned streetSegmentId);
    void pushIntersection(const unsigned intersectionId);
    
    // Creates unordered map that maps streets with which this street intersects
    // to the intersection IDs of those intersections. Deletes all allocated 
    // memory at the end of use.
    void setIntersectionStreets();
    
private:
    
    unsigned streetId;
    std::vector<unsigned> streetSegments;
    std::vector<unsigned> intersections;
    std::unordered_map<std::string, std::vector<unsigned>> intersectionStreets;
};

#endif /* STREET_H */

