#include "hotel_grasp.h"

bool tour_grc(uint trips_n, uint hotels_n, uint pois_n, uint rcl,
              vertex *v, double **d_matrix, trip *tour) {
    uintvec_endadd(&tour[0].list, 0);
    v[0].vis = true;
    uintvec_endadd(&tour[trips_n-1].list, 1);
    v[1].vis = true;

    vervec cndt_h;
    vervec_init(&cndt_h, hotels_n - 2);
    for (uint i = 2; i < hotels_n; ++i) vervec_endadd(&cndt_h, v[i]);

    for (uint i = 0; i < trips_n - 1; ++i) {
        for (uint j = 0; j < cndt_h.len; ++j) {
            cndt_h.items[j].tmp_score = d_add(tour[i].list.items[0], 1, cndt_h.items[j].idx, d_matrix);
            // printf("hotel%d: %.2f\n", cndt_h.items[j].idx, cndt_h.items[j].tmp_score);
        }
        if (rcl > cndt_h.len - 1) rcl = cndt_h.len - 1; // DEBUG: INVESTIGATE THIS LINE WHEN ABLE
        qsort(cndt_h.items, cndt_h.len, sizeof(vertex), vertex_cmp);

        // for (uint j = 0; j < cndt_h.len; ++j)
        //     printf("hotel%d: %.2f\n", cndt_h.items[j].idx, cndt_h.items[j].tmp_score);

        uint choice = (rcl > 0) ? rand()%rcl : 0;
        uint pos = cndt_h.len - 1 - choice;
        uintvec_endadd(&tour[i].list, cndt_h.items[pos].idx);
        uintvec_endadd(&tour[i+1].list, cndt_h.items[pos].idx);
        vervec_pop(&cndt_h, pos);

        // printf("trip %u: selected hotel %u based on choice %u/%u\n\n",
        //         i, tour[i].list.items[1], choice+1, rcl);

        tour[i].rem_len = tour[i].tot_len - d_matrix[tour[i].list.items[0]][tour[i].list.items[1]];
        // printf("tour%u remaining length is %.2f\n", i, tour[i].rem_len);
        if (tour[i].rem_len <= 0.0) return true;
    }

    return false;
}