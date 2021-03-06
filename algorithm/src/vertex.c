#include "vertex.h"

int vertex_cmp_desc(const void *a, const void *b){
    // compares two vertices by their tmp_score attribute in a descending manner.
    const vertex *aa = (const vertex *) a;
    const vertex *bb = (const vertex *) b;

    if (isinf(aa->tmp_score) || isnan(aa->tmp_score)) {
        if (isinf(bb->tmp_score) || isnan(bb->tmp_score))
            return 0;
        return -1;
    }
    if (isinf(bb->tmp_score) || isnan(bb->tmp_score)) {
        return 1;
    }

    if (aa->tmp_score < bb->tmp_score) return 1;
    if (aa->tmp_score > bb->tmp_score) return -1;
    return 0;
}
int vertex_cmp_asc(const void *a, const void *b) {
    // compares two vertices by their tmp_score attribute in an ascending manner.
    return vertex_cmp_desc(b, a);
}
double d(vertex a, vertex b) {
    // returns the distance between two vertices.
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}
double d_add(uint v1, uint v2, uint add_v, double **d_matrix) {
    // returns the distance added when adding a vertex between two others.
    return (d_matrix[v1][add_v] + d_matrix[add_v][v2] - d_matrix[v1][v2]);
}
void create_d_matrix(double **d_matrix, uint size, vertex *v) {
    // fills a distances matrix with all the required distances.
    for (uint i = 0; i < size; ++i) {
        for (uint j = 0; j < size; ++j) {
            if (i == j) d_matrix[i][j] = 0.0;
            else d_matrix[i][j] = d(v[i], v[j]);
        }
    }
    return;
}
double calc_h_score(uint index, uint hotels_n, uint pois_n, double importance,
    // calculates a hotel's "score" by taking into account its proximity to
    //      high-score pois.
    vertex *v, double **d_matrix) {
    double score = 0.0;
    for (uint j = hotels_n; j < hotels_n + pois_n; ++j) {
        if (d_matrix[index][j] <= 1.0) score += v[j].score;
        else score += importance * (v[j].score / d_matrix[index][j]);
    }
    return score;
}
