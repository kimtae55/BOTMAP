#ifndef M4_HELPER_H
#define M4_HELPER_H

#include <vector>
#include "DeliveryLocationTimes.h"

vector<unsigned> optimize(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty);
vector<unsigned> startTwoOpt(vector<unsigned> deliveryOrder, const std::vector<DeliveryInfo>& deliveries,  const std::vector<unsigned>& depots);
vector<unsigned> twoOpt(int i, int j, vector<unsigned> deliveryOrder,  const std::vector<unsigned>& depots);
double calculateTime(vector<unsigned> deliveryOrder);
vector<unsigned> reconstruct_path(vector<unsigned> deliveryOrder, const float turn_penalty);
bool check_legal(vector<unsigned> path, const std::vector<DeliveryInfo>& deliveries,  const std::vector<unsigned>& depots);
vector<unsigned> findGreedyPath(unsigned startDepot, unsigned endDepot, const vector<DeliveryInfo>& deliveries);

#endif /* M4_HELPER_H */

