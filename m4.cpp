#include "m4.h"
#include <unordered_map>
#include <vector>
#include <chrono>
#include <algorithm>
#include <list>

#include "DeliveryLocationTimes.h"
#include "m4.h"
#include "m4_helper.h"
#include "m3.h"

extern DeliveryLocationTimes deliveryMap;

using namespace std;

vector<unsigned> traveling_courier(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty) {
    

//    return findGreedyPath(depots[0], depots[0], deliveries);
    return optimize(deliveries, depots, turn_penalty);
}