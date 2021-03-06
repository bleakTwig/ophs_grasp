#ifndef GRASP_HOTEL_H
#define GRASP_HOTEL_H

#include "vertex.h"
#include "trip.h"

bool tour_grc(uint trips_n, uint hotels_n, uint pois_n, uint rcl,
              vertex *v, double **d_matrix, trip *tour);

#endif
