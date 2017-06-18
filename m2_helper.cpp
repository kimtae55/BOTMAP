#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>

#include "m1.h"
#include "helper_functions.h"
#include "Street.h"
#include "StreetsDatabaseAPI.h"
#include "m2.h"
#include "m2_helper.h"
#include "graphics.h"
#include "LatLon.h"
#include "Surface.h"
#include "OSMDatabaseAPI.h"
#include "global_variables.h"
#include <time.h>
#include "m3.h"
#include "LatLon.h"

using namespace std;

#define RADIANS_TO_DEGREE 180/PI

#define BOOKMARK_JUMP_OFFSET 0.005

t_point clickedPoint;
 t_point drawStartPosition;

unsigned currentIntersection;
bool positionSet = false;
bool showpoi = false;
bool showpoiname = false;
bool showminor = false;
bool drawhelp = false;
vector<unsigned> path;
bool drawpath = false;
bool selectIntersectionsOnMap = false;
bool selectIntersectionsCount = 0;

void draw_screen() {
   
    clearscreen();
 
    int zoomLevel = find_zoom_level();

    draw_features(zoomLevel);

    draw_streets(zoomLevel);
    
    draw_point_of_interest(zoomLevel);
    
    if (drawpath) {
       
        draw_path(path,zoomLevel,lastIntersectionId);
        draw_start_ping();
    }
    
    draw_street_name(zoomLevel);
    
    // Draws the help box if drawhelp is true.
    draw_helper();
    
    // Maintains the current setting of the position marker if its position is 
    // currently set.
    if (positionSet) {
        when_not_clicked();
    }
    
    
    
//    time_t endDrawTime;
//    endDrawTime = time(NULL);
//    double difference = double(difftime(endDrawTime, startDrawTime));
//    cout << "time taken to draw: " << difference << endl;
    
    copy_off_screen_buffer_to_screen(); 
    
 
}

void draw_start_ping() {
        setcolor(RED);
        fillarc(drawStartPosition.x,  drawStartPosition.y, 0.008, 0, 360);
}
// Draws all streets for the map.
void draw_streets(int zoomLevel)
{
    float average_latitude = float(currentMinMax.avgRadLat);
 
    vector<unsigned> streetSegments;

    for (unsigned i = 0; i < getNumberOfIntersections(); i++) 
    {
        streetSegments = find_intersection_street_segments(i);
        for (unsigned j = 0; j < streetSegments.size(); j++) 
        {
            if (alreadyDrawn[streetSegments[j]] == false) 
            {
                // Gets the latitude and longitude of the two intersections that
                // the street segment connects, converts them to a Cartesian
                // map and draws a line connecting the two intersections (i.e. 
                // draws the street segment).
                struct StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegments[j]);
                LatLon from = IntersectionData[segInfo.from].position;
                LatLon to;
                
                // setStreetProperties returns true if the street should be 
                // drawn at this zoom level.
                if (setStreetProperties(zoomLevel, streetSegments[j])) {
                    for (unsigned k = 0; k < segInfo.curvePointCount; k++) 
                    {
                        to = getStreetSegmentCurvePoint(streetSegments[j], k);
                        auto x1 = longitude_to_cartesian(from.lon(), average_latitude);
                        auto x2 = longitude_to_cartesian(to.lon(), average_latitude);
                        drawline(x1, from.lat(), x2, to.lat());
                        from = to;
                    }

                    to = IntersectionData[segInfo.to].position;
                    auto x1 = longitude_to_cartesian(from.lon(), average_latitude);
                    auto x2 = longitude_to_cartesian(to.lon(), average_latitude);
                    drawline(x1, from.lat(), x2, to.lat());
                }
            }
        }
    }
}

void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info) {
    // when re-clicked on an intersection or any other point, clear the previous mark
    // make it so that the mark works for all map
    // make it so that mark stays when zoomed or zoomed out
   
    auto const lon = cartesian_to_longitude(x, currentMinMax.avgRadLat);
    auto const lat = y;

    LatLon position(lat, lon);
    // need to be different in terms of zoom level...
    float bound = 0.0001;

    auto const id = find_closest_intersection(position);
    LatLon intersection_pos = getIntersectionPosition(id);

    float x_inter = longitude_to_cartesian(intersection_pos.lon(), currentMinMax.avgRadLat);
    float y_inter = intersection_pos.lat();
    if ((x > x_inter - bound) && (x < x_inter + bound) 
        && (y > y_inter - bound) && (y < y_inter + bound)) {
        // erase previous and highlight
        // RE DRAW THE CURRENT SCREEN.... --> need zooming level to be done!
        unsigned intersectionSelectionForPathFinding = 0;
        bool displayPath = false;
        if (selectIntersectionsOnMap) {
            if (selectIntersectionsCount < 1) {
                selectIntersectionsCount++;
            }
            else {
                selectIntersectionsCount = 0;
                selectIntersectionsOnMap = false;
                intersectionSelectionForPathFinding = currentIntersection;
                displayPath = true;
            }
        }
        highlightAndDisplayIntersection(x_inter, y_inter);
        cout << "Intersection information: " << IntersectionData[id].name << endl;
        currentIntersection = id;
        clickedPoint.x = x;
        clickedPoint.y = y;
        positionSet = true;
        if (displayPath) {
            path = find_path_between_intersections(intersectionSelectionForPathFinding, currentIntersection, 15);
            draw_path(path,find_zoom_level(),lastIntersectionId);
            determine_directions(path,lastIntersectionId);
            drawpath = true;
            displayPath = false;
        }
    }
    else {
        positionSet = false;
        drawpath = false;
    }
    draw_screen();

}

void when_not_clicked () {
    // means that either i'm doing nothing, or i'm zooming in 
    // keep drawing the surface at the same location so that it doesnt disappear
    float average_latitude = float(currentMinMax.avgRadLat);
    auto const lon = cartesian_to_longitude(clickedPoint.x, average_latitude);
    auto const lat = clickedPoint.y;

    LatLon position(lat, lon);
    // need to be different in terms of zoom level...
    float bound = 0.0001;

    auto const id = find_closest_intersection(position);
    LatLon intersection_pos = getIntersectionPosition(id);

    float x_inter = longitude_to_cartesian(intersection_pos.lon(), average_latitude);
    float y_inter = intersection_pos.lat();
    if ((clickedPoint.x > x_inter - bound) && (clickedPoint.x < x_inter + bound) 
        && (clickedPoint.y > y_inter - bound) && (clickedPoint.y < y_inter + bound)) {
        Surface image = load_png_from_file("libstreetmap/resources/locationpin.png");
        draw_surface(image, x_inter, y_inter);
    }
}

//----------------------------------------------------------------------------//    
// highlight the intersection and display info
//----------------------------------------------------------------------------//
void highlightAndDisplayIntersection(float x_inter, float y_inter) {
    // highlight intersection (can use a .png later)
    Surface image = load_png_from_file("libstreetmap/resources/locationpin.png");
    draw_surface(image, x_inter, y_inter);

}


float cartesian_to_longitude(float x, float average_latitude) {
    return (x / cos(average_latitude));
}

float longitude_to_cartesian(float longitude, float average_latitude) {
    return (longitude * cos(average_latitude));
}

//Auto zoom in the highlight all the intersections 
//when user input two street names in the terminal.
void find_button_function(void (*drawscreen) (void))
{
    float average_latitude = float(currentMinMax.avgRadLat);

    string street_name1, street_name2;
    cout << "Enter the first street name" << endl;
    getline(cin, street_name1);
    cout << "Enter the second street name" << endl;
    getline(cin, street_name2);
    
    vector<unsigned> foundIntersections = find_intersection_ids_from_street_names(street_name1, street_name2);
    if (foundIntersections.size() == 0) 
    {
        cout << "No intersections" << endl;
    }
    float x;
    float y;
    for (vector<unsigned>::iterator i = foundIntersections.begin();
            i != foundIntersections.end();i++) 
    {
        LatLon position = getIntersectionPosition(*i);
        x = longitude_to_cartesian(position.lon(), average_latitude);
        y = position.lat();
        int zoomLevel = find_zoom_level();
        
        // zoom in if current zoom-in level is larger 
        // than the middle-point zoom-in scale, and zoom out otherwise.
        if(zoomLevel > 4)
            auto_zoom_in(x,y,true);
        else if (zoomLevel < 4)
            auto_zoom_in(x,y,false);
       
        draw_screen();
        highlightAndDisplayIntersection(x, y);
        cout << "The intersection id is " << *i << " between " << getIntersectionName(*i) << endl;
    }
    clickedPoint.x = x;
    clickedPoint.y = y;
    // set this bool to true so the highlight does not disappear when zoom in and out.
    positionSet = true;
    
}

void switch_map_button(void(*drawscreen) (void)) {
    
    // clear buttons, etc. out before re-loading a map
    
    // close map, data structures
    close_map();
    // get input 
    string city;
    cout << "List of Cities:\n"
            "1. Toronto, Canada \n"
            "2. Beijing, China \n"
            "3. Cairo, Egypt \n"
            "4. Cape Town, South Africa \n"
            "5. Golden Horseshoe, Canada \n"
            "6. Hamilton, Canada \n" 
            "7. Hong Kong, China \n"
            "8. Iceland \n"
            "9. London, England \n"
            "10. Moscow, Russia \n"
            "11. New Delhi, India \n"
            "12. New York, USA \n"
            "13. Rio de Janeiro, Brazil \n"
            "14. Saint Helena \n"
            "15. Singapore \n"
            "16. Sydney, Australia \n"
            "17. Tehran, Iran \n"
            "18. Tokyo, Japan \n"
            "Enter number of city to switch: ";
    int cityNumber;
    cin >> cityNumber;
    
    string new_map_path;
    string new_osm_path;
    
    switch(cityNumber) {
        case 1:
        {   new_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/toronto_canada.osm.bin";
            update_message("Map of Toronto, Canada");
            break;
        }
        case 2:
        {
            new_map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/beijing_china.osm.bin";
            update_message("Map of Beijing, China");
            break;
        }
        case 3:
        {
            new_map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";  
            new_osm_path = "/cad2/ece297s/public/maps/cairo_egypt.osm.bin";
            update_message("Map of Cairo, Egypt");
            break;
        }
        case 4:
        {
            new_map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/cape-town_south-africa.osm.bin";
            update_message("Map of Cape Town, South Africa");
            break;
        }
        case 5:
        {
            new_map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.osm.bin";
            update_message("Map of Golden Horseshoe, Canada");
            break;
        }
        case 6:
        {
            new_map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/hamilton_canada.osm.bin";
            update_message("Map of Hamilton, Canada");
            break;
        }
        case 7:
        {
            new_map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/hong-kong_china.osm.bin";
            update_message("Map of Hong Kong, China");
            break;
        }
        case 8:
        {
            new_map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/iceland.osm.bin";
            update_message("Map of Iceland");
            break;
        }
        case 9:
        {
            new_map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/london_england.osm.bin";
            update_message("Map of London, England");
            break;
        }
        case 10:
        {
            new_map_path = "/cad2/ece297s/public/maps/moscow_russia.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/moscow_russia.osm.bin";
            update_message("Map of Moscow, Russia");
            break;
        }
        case 11:
        {
            new_map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/new-delhi_india.osm.bin";
            update_message("Map of New Delhi, India");
            break;
        }
        case 12:
        {
            new_map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/new-york_usa.osm.bin";
            update_message("Map of New York, USA");
            break;
        }
        case 13:
        {
            new_map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.osm.bin";
            update_message("Map of Rio de Janeiro, Brazil");
            break;
        }
        case 14:
        {
            new_map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/saint-helena.osm.bin";
            update_message("Map of Saint Helena");
            break;
        }
        case 15:
        {
            new_map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/singapore.osm.bin";
            update_message("Map of Singapore");
            break;
        }
        case 16:
        {
            new_map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/sydney_australia.osm.bin";
            update_message("Map of Sydney, Australia");
            break;
        }
        case 17:
        {
            new_map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/tehran_iran.osm.bin";
            update_message("Map of Tehran, Iran");
            break;
        }
        case 18:
        {
            new_map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/tokyo_japan.osm.bin";
            update_message("Map of Tokyo, Japan");
            break;
        }
        default:
        {
            cout << "Invalid city number. Loading default map.";
            new_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
            new_osm_path = "/cad2/ece297s/public/maps/toronto_canada.osm.bin";
            update_message("Map of Toronto, Canada");
            break;
        }
    }

    load_osm(new_osm_path);
    bool load_success = load_map(new_map_path);
    if(!load_success) {
        cerr << "Failed to load map '" << new_map_path << "'\n";
        cerr << "Exiting..." << endl;
        exit(1);
    }
    set_visible_world(currentMinMax.minLon * cos(currentMinMax.avgRadLat),
                      currentMinMax.minLat,
                      currentMinMax.maxLon * cos(currentMinMax.avgRadLat),
                      currentMinMax.maxLat);
    currentMinMax.maxBox = get_visible_world();
    cout << "Now drawing map!" << endl;
    draw_screen();
}

//Turn on and off all the poi with their names.
void ShowPOI_button(void (*drawscreen) (void)) {
    

    showpoi = !showpoi;
    drawscreen();
}

void bookmark_button(void (*drawscreen) (void)) {
    
    // If the user has marked an intersection, asks to bookmark it.
    if (positionSet) {
        cout << "Set new bookmark (Y/N)? ";
        char yesOrNo;
        cin >> yesOrNo;
        yesOrNo = toupper(yesOrNo);
        while (yesOrNo != 'Y' && yesOrNo != 'N') {
            cout << "Invalid input." << endl;
            cout << "Set new bookmark (Y/N)? ";
            cin >> yesOrNo;
            yesOrNo = toupper(yesOrNo);
        }
        if (yesOrNo == 'Y') {
            listOfBookmarks.push_back(currentIntersection);
            cout << "New bookmark set." << endl;
            return;
        }
    }
    if (listOfBookmarks.size() == 0) {
        cout << "No bookmarks." << endl;
    }
    else {
        int bookmarkNumber;
        cout << "List of bookmarks:" << endl;
        for (unsigned i = 0; i < listOfBookmarks.size(); i++) {
            cout << i + 1 << ". " << getIntersectionName(listOfBookmarks[i]) 
                    << endl;
        }
        // Asks to erase a bookmark from the list.
        cout << "Erase a bookmark (Y/N)? ";
        char yesOrNo;
        cin >> yesOrNo;
        yesOrNo = toupper(yesOrNo);
        while (yesOrNo != 'Y' && yesOrNo != 'N') {
            cout << "Invalid input." << endl;
            cout << "Erase a bookmark (Y/N)? ";
            cin >> yesOrNo;
            yesOrNo = toupper(yesOrNo);
        }
        if (yesOrNo == 'Y') {
            cout << "Select a bookmark to erase: ";
            cin >> bookmarkNumber;
            listOfBookmarks.erase(listOfBookmarks.begin() + bookmarkNumber - 1);
            cout << "Bookmark " << bookmarkNumber << " erased." << endl;
        }
        // Jumps to bookmark number if not erasing a bookmark.
        else {
            cout << "Select a bookmark number: " << endl;
            cin >> bookmarkNumber;
            clickedPoint.x = longitude_to_cartesian(getIntersectionPosition
                    (listOfBookmarks[bookmarkNumber-1]).lon(), currentMinMax.avgRadLat);
            clickedPoint.y = getIntersectionPosition(listOfBookmarks[bookmarkNumber-1]).lat();
            // Sets world to be zoomed in and centered at bookmark.
            set_visible_world(clickedPoint.x - BOOKMARK_JUMP_OFFSET,
                              clickedPoint.y - BOOKMARK_JUMP_OFFSET,
                              clickedPoint.x + BOOKMARK_JUMP_OFFSET,
                              clickedPoint.y + BOOKMARK_JUMP_OFFSET);
            cout << "Jumping to bookmark " << bookmarkNumber << endl;
            highlightAndDisplayIntersection(clickedPoint.x, clickedPoint.y);
            positionSet = true;
            drawscreen();
        }
    }
}

//Turn on and off all the poi names.
void ShowPOIname_button(void (*drawscreen) (void)) {
    showpoiname = !showpoiname;
    drawscreen();
}

//Turn on and off all the minor roads with their names.
void ShowMinor_button(void (*drawscreen) (void)) {
    

    showminor = !showminor;
    drawscreen();
}

//Called in draw_feature to draw polygon typed features.
void draw_polygon(unsigned feature_id, unsigned index, t_color colour) {

    unsigned numPoints = getFeaturePointCount(feature_id);
    setcolor(colour);
    t_point* pointsToDraw = new t_point[numPoints];
    for (unsigned i = 0; i < numPoints; i++) {
        pointsToDraw[i] = featureMap[index].featurePoints[i];
    }
    fillpoly(pointsToDraw, numPoints);
    delete [] pointsToDraw;
}

//Called in draw_feature to draw line typed features.
void draw_line(unsigned feature_id, unsigned index, color_types colour, int linewidth) {
    

    setlinewidth(linewidth);
    setcolor(colour);
  
    for (unsigned feature_point_id = 0; feature_point_id < getFeaturePointCount(feature_id)-1; feature_point_id++) {
       
        drawline(featureMap[index].featurePoints[feature_point_id], featureMap[index].featurePoints[feature_point_id+1]);
    }
}


void set_min_max() {
    
    // Sets the minimum and maximum latitudes and longitudes, along with other
    // useful information.
    
    currentMinMax.maxLat = getIntersectionPosition(0).lat();
    currentMinMax.minLat = currentMinMax.maxLat;
    currentMinMax.maxLon = getIntersectionPosition(0).lon();
    currentMinMax.minLon = currentMinMax.maxLon;
    
    IntersectionData.resize(getNumberOfIntersections());
    for (unsigned id = 0; id < getNumberOfIntersections(); id++) {
        IntersectionData[id].position = getIntersectionPosition(id);
        IntersectionData[id].name = getIntersectionName(id);
        
        currentMinMax.maxLat = max(currentMinMax.maxLat, IntersectionData[id].position.lat());
        currentMinMax.minLat = min(currentMinMax.minLat, IntersectionData[id].position.lat());
        
        currentMinMax.maxLon = max(currentMinMax.maxLon, IntersectionData[id].position.lon());
        currentMinMax.minLon = min(currentMinMax.minLon, IntersectionData[id].position.lon());
    }
    // Sets the average latitude in radians.
    currentMinMax.avgRadLat = 0.5 * DEG_TO_RAD * (currentMinMax.maxLat + currentMinMax.minLat);
}

void draw_features(int zoomLevel) 
{   
    // There are 4 layers of zoom in level to draw different types of features.
    for (unsigned i = 0; i < getNumberOfFeatures(); i++) 
    {
        // Draw closed feature. Layer 1 features include lake, island and so on.
        if (featureMap[i].isClosed) 
        {
            if (featureMap[i].featureType == Lake) {
                draw_polygon(featureMap[i].featureId, i, t_color(0xA3, 0xCC, 0xFF));
            }   
            else if (featureMap[i].featureType == Island) {
                draw_polygon(featureMap[i].featureId, i, t_color(0xF0, 0xED, 0xE5));
            }   
            else if (featureMap[i].featureType == Unknown) {
                draw_polygon(featureMap[i].featureId, i, t_color(222,210,190));
            }
            else if (featureMap[i].featureType == River) {
                draw_polygon(featureMap[i].featureId, i, t_color(0xA3, 0xCC, 0xFF));
            }        
            else if (featureMap[i].featureType == Greenspace&& zoomLevel <= 6) {
               draw_polygon(featureMap[i].featureId, i, t_color(0xB4, 0xD9, 0x80));   
            }
            else if (featureMap[i].featureType == Park) {
                draw_polygon(featureMap[i].featureId, i, t_color(0xCB,0xE6,0xA3)); 
            }
            else if (featureMap[i].featureType == Beach) {     
               draw_polygon(featureMap[i].featureId, i, t_color(0xFA, 0xF2, 0xC7));      
            } 
            // Layer 2 features that shown when zoomed in to zoom level <= 6.
            else if (featureMap[i].featureType == Greenspace&& zoomLevel <= 6) {
               draw_polygon(featureMap[i].featureId, i, t_color(0xD6, 0xE9, 0xB9));
            }
            // Layer 3 features that shown when zoomed in to zoom level <= 4.
            else if (featureMap[i].featureType == Golfcourse && zoomLevel <= 4) {     
                draw_polygon(featureMap[i].featureId, i, t_color(0x85, 0xDE, 0xAD));
            }
            // Layer 4 features that shown when zoomed in to zoom level <= 3.
            else if (featureMap[i].featureType == Building && zoomLevel <= 3) {
                draw_polygon(featureMap[i].featureId, i, t_color(0xD5, 0xD5, 0xD5));
            }             
        }
        else 
        {
            // Draw line typed features. Shoreline shows at top zoom level.
            if (featureMap[i].featureType == Shoreline) {
                int linewidth = 6;
                draw_line(featureMap[i].featureId, i, BLUE, linewidth);
            } 
            // Stream belongs to Layer 3 features that 
            // shown when zoomed in to zoom level <= 4.
            if (featureMap[i].featureType == Stream && zoomLevel <= 4) {
                int linewidth = 1;
                draw_line(featureMap[i].featureId, i, LIGHTSKYBLUE, linewidth);
            } 
        }
    }
}

// Return the current zoom level. There are 8 different zoom levels.
int find_zoom_level() {
    
    float longitudeRange = abs(get_visible_world().right() - get_visible_world().left());
    longitudeRange = cartesian_to_longitude(longitudeRange, currentMinMax.avgRadLat);
    float distance = longitudeRange * EARTH_RADIUS_IN_METERS * DEG_TO_RAD;
    if (distance < 1000) {
        return 1;
    }
    if (distance < 2500) {
        return 2;
    }
    else if (distance < 5000) {
        return 3;
    }
    else if (distance < 10000) {
        return 4;
    }
    else if (distance < 20000) {
        return 5;
    }
    else if (distance < 50000) {
        return 6;
    }
    else if (distance < 100000) {
        return 7;
    }
    else {
        return 8;
    }
    
}

//Set the line width based on current zoom level.
int find_line_width(int zoomLevel) {
    
    switch (zoomLevel) {
        case 1: return 16;
        case 2: return 9;
        case 3: return 7;
        case 4: return 5;
        case 5: return 5;
        case 6: return 3;
        case 7: return 2;
        default: return 1;
    }
}

// Draw 3 layers of points of interest based on current zoom level.
void draw_point_of_interest(int zoomLevel)
{
    // if POI is not disabled, draw them.
    if(!showpoi)
    {
        float average_latitude = float(currentMinMax.avgRadLat);
        
        for (unsigned pointOI_id = 0; pointOI_id < getNumberOfPointsOfInterest(); pointOI_id++) 
        {
            settextattrs(10, 0);
            string name = getPointOfInterestName(pointOI_id);
            string type = getPointOfInterestType(pointOI_id);
            LatLon position = getPointOfInterestPosition(pointOI_id);
            // First layer POI for zoom level <= 4.
            if(zoomLevel<= 4)
            {
                if(type == "post_office" || type == "hospital" || 
                type == "supermarket" || type == "police" )                              
                {                                                                              
                   setcolor(ORANGE);
                   fillarc(position.lon()*cos(average_latitude), position.lat(), 0.00008, 0, 360);
                   // if POI name is not disabled, show the names.
                   if(!showpoiname)
                    {
                    setcolor(BLACK);
                    drawtext(position.lon()*cos(average_latitude), position.lat()
                             + 0.00015, name, 10, 10);
                    }
                
                }
            }
            // Second layer POI for zoom level <= 3.
            if(zoomLevel<= 3)
            {
                if(type == "food_court" || type == "convenience"
                    || type == "fuel" || type == "cinema" || type == "general")                               
                {                                                                               
                   setcolor(YELLOW);
                   fillarc(position.lon()*cos(average_latitude), position.lat(), 0.00008, 0, 360);
                   // if POI name is not disabled, show the names.
                   if(!showpoiname)
                    {
                    setcolor(BLACK);
                    drawtext(position.lon()*cos(average_latitude), position.lat()
                             + 0.00015, name, 10, 10);
                    }
                }
            }
            // Third layer POI for zoom level <= 2.
            if(zoomLevel<= 2)
            {
                if(type == "coffee" || type == "bar" ||type == "beauty" ||
                 type == "atm" || type == "clothes" || type == "bakery")                               
                {                                                                               
                   setcolor(PLUM);
                   fillarc(position.lon()*cos(average_latitude), position.lat(), 0.00008, 0, 360);
                   // if POI name is not disabled, show the names.
                   if(!showpoiname)
                    {
                    setcolor(BLACK);
                    drawtext(position.lon()*cos(average_latitude), position.lat()
                             + 0.00015, name, 10, 10);
                    }
                }
            }    
        }  
    }
} 

bool setStreetProperties(int zoomLevel, unsigned streetSegment) {
    
    // Sets the properties of the street depending on the type of street it is.
    // Properties set include the line width and colour. Also determines whether
    // the street should be drawn at this level.
    if (streetTypeMap[streetSegment] == "motorway" || streetTypeMap[streetSegment] == "primary" || 
            streetTypeMap[streetSegment] == "trunk") {
        setlinewidth(find_line_width(zoomLevel)+2);
        setcolor(t_color(0xFE,0xD8,0x9D));
        return true;
    }
    else if (streetTypeMap[streetSegment] == "tertiary" || streetTypeMap[streetSegment] == "unclassified") 
    {
        setcolor(WHITE);
        if (zoomLevel <=5 ) 
        {
            setlinewidth(find_line_width(zoomLevel)-1);
            return true;
        }
        else 
        {
            return false;
        }  
    }
    else if (streetTypeMap[streetSegment] == "trunk_link") {
        setlinewidth(find_line_width(zoomLevel));
        setcolor(t_color(0xFD,0xE7,0xC4));
        if (zoomLevel <=5 ) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (streetTypeMap[streetSegment] == "primary_link") {
        setlinewidth(find_line_width(zoomLevel));
        setcolor(t_color(0xFD,0xE7,0xC4));
        if (zoomLevel <=5 ) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (streetTypeMap[streetSegment] == "motorway_link") {
        setlinewidth(find_line_width(zoomLevel));
        setcolor(t_color(0xFD,0xE7,0xC4));
        if (zoomLevel <=5 ) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (streetTypeMap[streetSegment] == "secondary") {
        setlinewidth(find_line_width(zoomLevel));
        setcolor(WHITE);
        return true;
    }
    else 
    {
        setlinewidth(find_line_width(zoomLevel));
        setcolor(WHITE);
        if (zoomLevel <= 4) 
        {
            if(!showminor)
            {
                setlinewidth(find_line_width(zoomLevel)-4);
                return true;
            }
            else 
            {
                return false;
            }
        }
    }
}

//----------------------------------------------------------------------------//
// This function draws Street Names according to the zoom level given. 
// It properly aligns streetNames to the street by calculating the angle 
// For curved ways the street name is drawn at the average midpoint of that curve
// For straight ways the street name is drawn at the midpoint.
// For one way streets names are distinguished in firebrick colour, and points 
// in the direction of the one-way.
//----------------------------------------------------------------------------//

void draw_street_name(int zoomLevel) {
    if (zoomLevel <= 2) {
        for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
            if (i%3 ==0 || i%5 == 0 || i%4 ==0) {
                t_point center;
                struct StreetSegmentInfo current = getStreetSegmentInfo(i);
                string name = getStreetName(current.streetID);
                string newName;
                IntersectionIndex start;
                IntersectionIndex end;
                LatLon startPosition;
                LatLon endPosition;
                float angle;

                // some constraints so that if either the text is too long
                // or there's too many same names per street, but it out
                // draw text here according to zoom levels
                if (name != "<unknown>" ) {
                    // positioning
                    

                    if (current.curvePointCount == 1) {
                        start = current.from;
                        startPosition = getIntersectionPosition(start);
                        endPosition = getStreetSegmentCurvePoint(i, 0);
                        double averageLon = (startPosition.lon() + endPosition.lon())/2.0;
                        double averageLat = (endPosition.lat() + startPosition.lat())/2.0;

                        center.x = longitude_to_cartesian(averageLon, currentMinMax.avgRadLat);
                        center.y = averageLat;
                        
                        float y = endPosition.lat() - startPosition.lat();
                        float x = endPosition.lon()*cos(currentMinMax.avgRadLat) - startPosition.lon()*cos(currentMinMax.avgRadLat);
                        angle = atan(y/x)*RADIANS_TO_DEGREE;
                        if (x == 0) {
                            angle = 0;
                        }     
                    }
                    else if (current.curvePointCount > 1) {

                        startPosition = getStreetSegmentCurvePoint(i, current.curvePointCount/2 -1);
                        endPosition = getStreetSegmentCurvePoint(i, current.curvePointCount/2);
                        double averageLon = (startPosition.lon() + endPosition.lon())/2.0;
                        double averageLat = (endPosition.lat() + startPosition.lat())/2.0;

                        center.x = longitude_to_cartesian(averageLon, currentMinMax.avgRadLat);
                        center.y = averageLat; 

                        float y = endPosition.lat() - startPosition.lat();
                        float x = endPosition.lon()*cos(currentMinMax.avgRadLat) - startPosition.lon()*cos(currentMinMax.avgRadLat);
                        angle = atan(y/x)*RADIANS_TO_DEGREE;
                        if (x == 0) {
                            angle = 0;
                        }                        
                    }

                    else {
                        start = current.from;
                        end = current.to;
                        // LatLon of a street seg
                        startPosition = getIntersectionPosition(start); 
                        // LatLon of the endpoint of that street seg
                        endPosition = getIntersectionPosition(end);

                        center.y = (startPosition.lat() + endPosition.lat())*0.5;
                        center.x = (startPosition.lon()*cos(currentMinMax.avgRadLat) 
                                   + endPosition.lon()*cos(currentMinMax.avgRadLat))*0.5;

                        float y = endPosition.lat() - startPosition.lat();
                        float x = endPosition.lon()*cos(currentMinMax.avgRadLat) - startPosition.lon()*cos(currentMinMax.avgRadLat);
                        angle = atan(y/x)*RADIANS_TO_DEGREE;
                        if (x == 0) {
                            angle = 0;
                        }                        
                    }        

                    // set angle
                    settextrotation(angle);

                    // some constraints so that if either the text is too long
                    // or there's too many same names per street, but it out
                    // draw text here according to zoom levels
                    float boundx = find_street_segment_length(i);

                    if (name != "<unknown>") {

                        // set color
                        if (current.oneWay) {
                            setcolor(BLACK);
                            if ( (startPosition.lat() < endPosition.lat() && startPosition.lon() > endPosition.lon()) ||
                                (startPosition.lat() > endPosition.lat() && startPosition.lon() > endPosition.lon()) ) {
                                newName = "<<<<<  " + name;
                            } 
                            else {
                                newName = name + "  >>>>>";
                            }        
                        }
                        else {
                            setcolor(BLACK);
                            newName = name;
                        }

                        if (streetTypeMap[i] == "motorway" || streetTypeMap[i] == "primary" || 
                            streetTypeMap[i] == "trunk") {

                            if(zoomLevel <= 3 && current.oneWay) {
                                setfontsize(7);
                                drawtext(center.x, center.y, newName, boundx, find_line_width(zoomLevel));

                            }
                            else if (zoomLevel <= 3) {
                                setfontsize(7);
                                drawtext(center.x, center.y, name, boundx, find_line_width(zoomLevel));
                            }
                        }
                        else if (streetTypeMap[i] == "secondary") {
                            if (zoomLevel <= 2 && current.oneWay) {
                                if (find_street_segment_length(i) > 100.0) {
                                    setfontsize(7);
                                    drawtext(center.x, center.y, newName, boundx, find_line_width(zoomLevel));
                                }
                            }
                            else if (zoomLevel <= 2) {
                                if (find_street_segment_length(i) > 100.0) {
                                    setfontsize(7);
                                    drawtext(center.x, center.y, newName, boundx, find_line_width(zoomLevel));
                                }
                            }
                        }
                        else 
                        {
                            if(zoomLevel <= 2 && current.oneWay) {
                                if(!showminor)
                                {
                                    if (find_street_segment_length(i) > 100.0) {
                                        setfontsize(7);
                                        drawtext(center.x, center.y, newName, boundx, find_line_width(zoomLevel));
                                    }
                                }
                            }
                        }     
                    }
                }
            }
        }
    }
}

//Turn on/off help menu.
void help_button(void (*drawscreen) (void)) {
    
    drawhelp = !drawhelp;
    drawscreen();

}
//Draw a half-transparent window that explains each special feature button.
void draw_helper()
{    
    if(drawhelp)
    {

    string find = "Find: Take two street names from user in terminal and highlight all intersections found on the map.";
    string switch_map = "Switch Map: Take a map file string from user and draw the according map.";
    string show_poi = "Show POI: Enable/disable all the points of interest. Three layers of POI represented in orange,yellow, and plum.";
    string poi_name = "POI Name: Enable/disable the names of points of interest.";
    string show_minor = "Show Minor: Enable/disable the minor roads with their names.";
    string bookmark = "Bookmark: Allow user to store intersections in a list and auto-zoom to any saved intersection.";
    string find_path_1 = "Find Path: User specify two intersections either by typing in two intersecting street names for each intersection" 
                       " or clicking two intersections on the map.";
    string find_path_2 = "A shortest valid path between those two intersections will be drawn.";
    string find_poi_path_1 = "Find POI Path: User first specifies a start intersection by typing in two intersecting street names, and then typing in a POI name.";
    string find_poi_path_2 = "The shortest valid path between the intersection and the closest POI with that name in terms of travel time will be drawn.";
    
    t_bound_box boundary = get_visible_world();

    t_point center = get_visible_world().get_center();
    float tot_height = get_visible_world().get_height();
    float height = tot_height / 13;

    settextrotation(0);
    setcolor(0,0,0,50);
    fillrect(boundary);
    
    setfontsize(12);
    setcolor(WHITE);
    
   
    drawtext(center.x, center.y + height * 5, find, 100, 100);
    drawtext(center.x, center.y + height * 4, switch_map, 100, 100);
    drawtext(center.x, center.y + height * 3, show_poi, 100, 100);
    drawtext(center.x, center.y + height * 2, poi_name, 100, 100);
    drawtext(center.x, center.y + height * 1, show_minor, 100, 100);
    drawtext(center.x, center.y + height * 0, bookmark, 100, 100);
    drawtext(center.x, center.y - height * 1, find_path_1, 100, 100);
    drawtext(center.x, center.y - height * 2, find_path_2, 100, 100);
    drawtext(center.x, center.y - height * 3, find_poi_path_1, 100, 100);
    drawtext(center.x, center.y - height * 4, find_poi_path_2, 100, 100);   
    }    
}  
 
void find_poi_path_button(void (*drawscreen)(void)) {

    // Should first clear the previous path drawn and any selected intersection.
    float average_latitude = float(currentMinMax.avgRadLat);    
    string street_name1,street_name2, POIname;
    unsigned n, intersect_id;
    int zoomLevel;
    float x,y;

    if (!path.empty())
        path.clear();

    //get intersecting street names from users
    cout << "Please enter the first street name: ";
    getline(cin, street_name1);
    cout << "Please enter the second street name: ";
    getline(cin, street_name2);

    vector<unsigned> intersect = find_intersection_ids_from_street_names(street_name1,street_name2);

        // No valid intersection
        if (intersect.size() == 0) {
            cout << "No valid intersections found. Please press button again.\n";
            return;}

        // Highlight and auto zoom-in to display all intersections found 
        for (unsigned i=0; i < intersect.size();i++)
        {
            cout << "The intersection ID of index " << i << " between those two streets is " << intersect[i] << endl;
            LatLon position = getIntersectionPosition(intersect[i]);

            x = longitude_to_cartesian(position.lon(), average_latitude);
            y = position.lat();
            zoomLevel = find_zoom_level();

        // zoom in if current zoom-in level is larger 
        // than the middle-point zoom-in scale, and zoom out otherwise.
        if(zoomLevel > 4)
            auto_zoom_in(x,y,true);
        else if (zoomLevel < 4)
            auto_zoom_in(x,y,false);  
        
        draw_screen();
        highlightAndDisplayIntersection(x, y);     
    }   
    
    clickedPoint.x = x;
    clickedPoint.y = y;
    positionSet = true;
    // set this bool to true so the highlight does not disappear when zoom in and out.
    

        if (intersect.size() == 1) 
            intersect_id = intersect[0];

        else if (intersect.size() > 1) {
           cout << "Please choose one intersection by specifying its number starting from 0 to " << intersect.size()-1 << endl; 
           cin >> n;
           while (n > intersect.size()-1){
              cout << "Invalid number. Please type in the number again." << endl;
              cin >> n;
           }
           intersect_id = intersect[n];
        } 
    cin.ignore(1000, '\n');
    cout << "Intersection selected as " << intersect_id << "\n" 
         << "Now please enter the name of a point of interest." << endl;
    getline(cin, POIname);
    auto iter = POIname_InterID.find(POIname);
    
    while (iter == POIname_InterID.end()){
        cout << "Invalid POI name, please re-enter the name.\n";
        cin.ignore(1000, '\n');
        getline (cin, POIname);
    }

    //Use 15 s turn penalty for default
    path = find_path_to_point_of_interest(intersect_id, POIname, 15);
    zoomLevel = find_zoom_level();
    draw_path(path,zoomLevel,lastIntersectionId);
    determine_directions(path,lastIntersectionId);

    drawpath = true;
}

void find_path_button(void (*drawscreen) (void)) {
    
    cout << "Would you like to select the intersections on the map? If not, you"
         << " may enter the intersections in the terminal. [Y/N] ";
    char inputChar;
    cin >> inputChar;
    while (inputChar != 'Y' && inputChar != 'N') {
        cout << "Invalid input. Please try again. ";
        cin.ignore(1000, '\n');
        cin >> inputChar;
    }
    if (inputChar == 'Y') {
        cout << "Please select the start and end intersections on the map." << endl;
        selectIntersectionsOnMap = true;
    }
    else {
        string street_name1;
        string street_name2;
        cin.ignore(1000, '\n');
        //get intersecting street names from users
        cout << "Please enter the first street name of the first intersection: ";
        getline(cin, street_name1);
        cout << "Please enter the second street name of the first intersection: ";
        getline(cin, street_name2);

        vector<unsigned> intersect = find_intersection_ids_from_street_names(street_name1,street_name2);

            // No valid intersection
        if (intersect.size() == 0) {
            cout << "No valid intersections found. Please press button again.\n";
            return;
        }
        float x, y;
        int zoomLevel;
        float average_latitude = float(currentMinMax.avgRadLat); 
        unsigned intersect_id1, intersect_id2, n;
        // Highlight and auto zoom-in to display all intersections found 
        for (unsigned i=0; i < intersect.size();i++)
        {
            cout << "The intersection ID of index " << i << " between those two streets is " << intersect[i] << endl;
            LatLon position = getIntersectionPosition(intersect[i]);

            x = longitude_to_cartesian(position.lon(), average_latitude);
            y = position.lat();
            zoomLevel = find_zoom_level();

            // zoom in if current zoom-in level is larger 
            // than the middle-point zoom-in scale, and zoom out otherwise.
            if(zoomLevel > 4)
                auto_zoom_in(x,y,true);
            else if (zoomLevel < 4)
                auto_zoom_in(x,y,false);  

            setcolor(RED);
            fillarc(position.lon()*cos(average_latitude), position.lat(), 0.00008, 0, 360);
        }   


        if (intersect.size() == 1) 
            intersect_id1 = intersect[0];

        else if (intersect.size() > 1) {
           cout << "Please choose one intersection by specifying its number starting from 0 to " << intersect.size()-1 << endl; 
           cin >> n;
           while (n > intersect.size()-1){
              cout << "Invalid number. Please type in the number again." << endl;
              cin >> n;
           }
           intersect_id1 = intersect[n];

        }
        cout << "Please enter the first street name of the second intersection: ";
        getline(cin, street_name1);
        cout << "Please enter the second street name of the second intersection: ";
        getline(cin, street_name2);

        intersect = find_intersection_ids_from_street_names(street_name1,street_name2);

            // No valid intersection
        if (intersect.size() == 0) {
            cout << "No valid intersections found. Please press button again.\n";
            return;
        }
        // Highlight and auto zoom-in to display all intersections found 
        for (unsigned i=0; i < intersect.size();i++)
        {
            cout << "The intersection ID of index " << i << " between those two streets is " << intersect[i] << endl;
            LatLon position = getIntersectionPosition(intersect[i]);

            x = longitude_to_cartesian(position.lon(), average_latitude);
            y = position.lat();
            zoomLevel = find_zoom_level();

            // zoom in if current zoom-in level is larger 
            // than the middle-point zoom-in scale, and zoom out otherwise.
            if(zoomLevel > 4)
                auto_zoom_in(x,y,true);
            else if (zoomLevel < 4)
                auto_zoom_in(x,y,false);  

            setcolor(RED);
            fillarc(position.lon()*cos(average_latitude), position.lat(), 0.00008, 0, 360);
        }   


        if (intersect.size() == 1) 
            intersect_id2 = intersect[0];

        else if (intersect.size() > 1) {
           cout << "Please choose one intersection by specifying its number starting from 0 to " << intersect.size()-1 << endl; 
           cin >> n;
           while (n > intersect.size()-1){
              cout << "Invalid number. Please type in the number again." << endl;
              cin >> n;
           }
           intersect_id2 = intersect[n];

        }
        path = find_path_between_intersections(intersect_id1, intersect_id2, 15);
        zoomLevel = find_zoom_level();
        draw_path(path,zoomLevel,lastIntersectionId);
        determine_directions(path,lastIntersectionId);
        drawpath = true;
    }
    drawscreen();
}

void start_graphics() {
    
    init_graphics("Welcome to BotMap", t_color(0xEA, 0xEA, 0xEA));
    
    set_drawing_buffer(OFF_SCREEN);
    
    // Create buttons on the side of the window.
    
    create_button("Window","Find",find_button_function);
    create_button("Find", "Switch Map", switch_map_button);
    create_button("Switch Map","Show POI",ShowPOI_button);
    create_button("Show POI","POI Name",ShowPOIname_button);
    create_button("POI Name","Show Minor",ShowMinor_button);
    create_button("Show Minor", "Bookmark", bookmark_button);
    create_button("Bookmark","Help",help_button);
    create_button("Help","Find POI Path",find_poi_path_button);
    create_button("Find POI Path", "Find Path", find_path_button);
    
    set_visible_world(currentMinMax.minLon * cos(currentMinMax.avgRadLat),
                      currentMinMax.minLat,
                      currentMinMax.maxLon * cos(currentMinMax.avgRadLat),
                      currentMinMax.maxLat);
    currentMinMax.maxBox = get_visible_world();
    
    update_message("Map of Toronto, Canada");
}

void determine_directions(vector<unsigned> travelPath, unsigned finalIntersection) {
    
    double distanceAlongStreet = find_street_segment_length(travelPath[0]);
    struct StreetSegmentInfo inStreet;
    struct StreetSegmentInfo outStreet;
    inStreet = getStreetSegmentInfo(travelPath[0]);
    cout << "Directions: " << endl;
    for (int i = 0; i < travelPath.size() - 1; i++) {
        outStreet = getStreetSegmentInfo(travelPath[i + 1]);
        if (inStreet.streetID == outStreet.streetID) {
            // If street ID is the same, then traveling along the same street, so
            // add distance current sum.
            distanceAlongStreet += find_street_segment_length(travelPath[i + 1]);
        }
        else {
            // Final distance along street has been calculated, so display it.
            cout << "Drive " << kilometres_or_metres(distanceAlongStreet) << "m on " << getStreetName(inStreet.streetID) << "." << endl;
            double crossProduct = cross_product_of_two_streets(inStreet, outStreet);
            // Determine cross product to see how to turn.
            if (crossProduct > 0) {
                cout << "Turn left ";
            }
            else if (crossProduct < 0) {
                cout << "Turn right ";
            }
            else {
                cout << "Continue ";
            }
            cout << "on " << getStreetName(outStreet.streetID) << "." << endl;
            distanceAlongStreet = find_street_segment_length(travelPath[i+1]);
        }
        inStreet = outStreet;
    }
    inStreet = getStreetSegmentInfo(travelPath[travelPath.size() - 2]);
    if (inStreet.streetID == outStreet.streetID) {
            // If street ID is the same, then the final distance would not be
            // displayed, so display it here.
            cout << "Drive " << kilometres_or_metres(distanceAlongStreet) << "m on " << getStreetName(inStreet.streetID) << "." << endl;
    }
    cout << "Arrive at intersection " << getIntersectionName(finalIntersection) << endl;
    
}

double round_nearest_ten_metres(double distance) {
    
    int integerDistance = int(distance);
    int leastSigDig = integerDistance % 10;
    if (leastSigDig >= 5) {
        return integerDistance + (10 - leastSigDig);
    }
    else {
        return integerDistance - leastSigDig;
    }
}

string kilometres_or_metres(double distance) {
    
    stringstream conversionStream;
    string stringToReturn;
    if (distance >= 1000) {
        // Convert to kilometres if distance is over 1000m.
        distance = round_nearest_ten_metres(distance);
        distance = distance / 1000;
        conversionStream << fixed << setprecision(1) << distance;
        conversionStream >> stringToReturn;
        stringToReturn += " k";
    }
    // Prevents displaying "Drive 0 m" by rounding up to 10m if less than 10m.
    else if (distance < 10) {
        distance = 10;
    }
    else {
        conversionStream << round_nearest_ten_metres(distance);
        conversionStream >> stringToReturn;
        stringToReturn += " ";
    }
    return stringToReturn;
}

double cross_product_of_two_streets(struct StreetSegmentInfo inStreetSeg, struct StreetSegmentInfo outStreetSeg) {
    
    LatLon commonIntersection, inStreetIntersection, outStreetIntersection;
    if (inStreetSeg.from == outStreetSeg.from || inStreetSeg.from == outStreetSeg.to) {
        commonIntersection = getIntersectionPosition(inStreetSeg.from);
        inStreetIntersection = getIntersectionPosition(inStreetSeg.to);
        outStreetIntersection = getIntersectionPosition(inStreetSeg.from == outStreetSeg.from ? outStreetSeg.to : outStreetSeg.from);
    }
    else {
        commonIntersection = getIntersectionPosition(inStreetSeg.to);
        inStreetIntersection = getIntersectionPosition(inStreetSeg.from);
        outStreetIntersection = getIntersectionPosition(inStreetSeg.to == outStreetSeg.from ? outStreetSeg.to : outStreetSeg.from);
    }
    double inStreetX = commonIntersection.lon() - inStreetIntersection.lon();
    double inStreetY = commonIntersection.lat() - inStreetIntersection.lat();
    double outStreetX = outStreetIntersection.lon() - commonIntersection.lon();
    double outStreetY = outStreetIntersection.lat() - commonIntersection.lat();
    return (inStreetX * outStreetY - inStreetY * outStreetX);
}
         
        
void draw_path(vector<unsigned> travelPath, int zoomLevel, unsigned finalIntersection) {
    // set it as the blue with transparency
    setcolor(66, 244, 241 ,150);
 
    float average_latitude = float(currentMinMax.avgRadLat);
 
    setlinewidth(find_line_width(zoomLevel)-2);
    for (unsigned j = 0; j < travelPath.size(); j++) 
    {
        // Gets the latitude and longitude of the two intersections that
        // the street segment connects, converts them to a Cartesian
        // map and draws a line connecting the two intersections (i.e. 
        // draws the street segment).
        struct StreetSegmentInfo segInfo = getStreetSegmentInfo(travelPath[j]);
        LatLon from = IntersectionData[segInfo.from].position;
        LatLon to;

        // setStreetProperties returns true if the street should be 
        // drawn at this zoom level.
        for (unsigned k = 0; k < segInfo.curvePointCount; k++) 
        {
            to = getStreetSegmentCurvePoint(travelPath[j], k);
            auto x1 = longitude_to_cartesian(from.lon(), average_latitude);
            auto x2 = longitude_to_cartesian(to.lon(), average_latitude);
            drawline(x1, from.lat(), x2, to.lat());
            from = to;
        }

        to = IntersectionData[segInfo.to].position;
        auto x1 = longitude_to_cartesian(from.lon(), average_latitude);
        auto x2 = longitude_to_cartesian(to.lon(), average_latitude);
        drawline(x1, from.lat(), x2, to.lat());
        
    }
        auto xOfFinalIntersection = longitude_to_cartesian(getIntersectionPosition(finalIntersection).lon(), average_latitude);
        highlightAndDisplayIntersection(xOfFinalIntersection, getIntersectionPosition(finalIntersection).lat());
        
}
