#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>
#include <string.h>

#include "vertex.h"
#include "trip.h"
#include "hotelGrasp.h"
#include "poiGrasp.h"
//#include "fileIO.h"
//  #ifndef GRASP_FILEIO_H
#include "userIO.h"

/* TODO: write a description for each function in a standard C way.
 * TODO: the tour construction algorithm has no encouragement to move around the
 *          solution space, causing many solutions to be curled up in a small
 *          area. Different construction algorithms could be proposed that favor
 *          exploration of the solution space.
 * TODO: tours should be a struct themselves, with a trips vector (or dynamic
 *          array) and a score.
 * TODO: trips should only contain a vertex's index, not the entire struct.
 * TODO: use shorter names, like v for the vertices array and t for the tours
 *          array, this will lead to less verbose and more readable code in the
 *          long run.
 * TODO: test separating the hotel selection to the poi selection in two diffe-
 *          rent for loops. It might work better. For the hotel selection, try
 *          maximizing the usable distance and the trip potential score (or ho-
 *          tel score if I'm lazy).
 * TODO: upgrade the file output so that it also shows the total score collected
 *          in each individual trip as well as for the whole tour.
 * TODO: separate this main.cpp into different files connected through .hpp
 *          and write a make to compile it all.
 */

void writeTours(unsigned int tripsSize, struct Trip *tour, const char* inFile) {
    std::ofstream out;
    out.open (inFile, std::ofstream::out | std::ofstream::trunc);
    for (unsigned int i = 0; i < tripsSize; ++i) {
        out << tour[i].start.index << " "; // write trip's start hotel
        for (unsigned int j = 0; j < tour[i].poiList.size(); ++j)
            out << tour[i].poiList[j].index << " "; // write trip's pois
        out << tour[i].end.index << "\n"; // write trip's end hotel
    }
    out.close();
    return;
}
int main(int argc, char* argv[]) {
    /* TODO: argument handling
     *  inFile with a default file chosen if none is given
     *  number of iterations with a default given by average time per iteration
     *  a flag for choosing between a random seed or a given one
     *  amount of iterations to be run
     *  HotelRCLSize with a default of 3
     *  PoiRCLSize with a default of 3
     *  Debug flag with a default of false, with 1 meaning normal and 2 meaning verbose
     */
    unsigned int iterNumber = 10;

    unsigned int HotelRCLSize = 3;
    unsigned int PoiRCLSize = 3;
    unsigned int debug = 0;

    srand(time(NULL)); // TODO: a flag should handle this

    const char *inFile;
    const char *outFile;
    if (argc == 3) {
        inFile  = argv[1];
        outFile = argv[2];
    }
    else {
        fprintf(stderr, "Usage: Exiting...\n"); // TODO: Usage()
        return 1;
    }

    if (!debug) printf("Running on %s...\n", inFile);

    // variables used
    unsigned int tripsSize;
    double *tripsLength;
    unsigned int hotelsSize;
    unsigned int poisSize;
    struct Vertex *vertices;
    // TODO: unsigned int unusedVertices MAYBE NOT REALLY NECESSARY;

// === FILE READING ============================================================
    { // file reading TODO: move to its own function maybe.
        std::ifstream infile(inFile);

        // parameters
        infile >> poisSize >> hotelsSize >> tripsSize;
        poisSize -= 2;
        hotelsSize += 2;

        // tour length
        double totalTourLength;
        infile >> totalTourLength;

        // trips length
        try {
            tripsLength = new double [tripsSize];
        } catch(std::bad_alloc) {
            fprintf(stderr, "Not enough memory allocated. Exiting...\n");
            return 1;
        }
        double distanceCheck = 0.0;
        for (unsigned int i = 0; i < tripsSize; ++i) {
            infile >> tripsLength[i];
            distanceCheck += tripsLength[i];
        }
        if (distanceCheck >= (totalTourLength * 1.02) ||
            distanceCheck <= (totalTourLength * 0.98)) {
            printf("The given total tour length doesn't correlate with the ");
            printf("length of each individual trip. The instance given is ");
            printf("wrong. Exiting...\n");
            return 1;
        }

        // hotels and points of interest
        try {
            vertices = new struct Vertex [hotelsSize + poisSize];
        } catch(std::bad_alloc) {
            printf("Not enough memory allocated. Exiting...\n");
            return 1;
        }
        for (unsigned int i = 0; i < hotelsSize + poisSize; ++i) {
            vertices[i].index = i;
            infile >> vertices[i].x >> vertices[i].y >> vertices[i].score;
        }
        infile.close();
    }
    if (debug) printf("===========================================\n");
    if (debug) printf("FILE READING OK!\n");
    if (debug == 2) printVariables (tripsSize, tripsLength,
                                    hotelsSize, poisSize, vertices);

// === MATRIX FILLING ==========================================================
    double **distancesMatrix;
    try {
        distancesMatrix = new double *[hotelsSize + poisSize];
        for (unsigned int i = 0; i < hotelsSize + poisSize; ++i)
            distancesMatrix[i] = new double[hotelsSize + poisSize];
    } catch(std::bad_alloc) {
        fprintf(stderr, "Not enought memory allocated. Exiting...");
        return 1;
    }
    fillMatrix(hotelsSize, poisSize, distancesMatrix);
    if (debug) printf("===========================================\n");
    if (debug) printf("MATRIX FILLING OK!\n");
    if (debug == 3) printMatrix(hotelsSize, poisSize, distancesMatrix);

// === HOTEL SCORE CALCULATION =================================================
    // TODO: picking hotels based on trip score
    calculateHotelScore(hotelsSize, poisSize, vertices, distancesMatrix);
    if (debug) printf("===========================================\n");
    if (debug) printf("HOTEL SCORE CALCULATION OK!\n");
    if (debug == 2) printHotelScores(hotelsSize, vertices);

    struct Trip *bestTour;
    try {
        bestTour = new struct Trip [tripsSize];
    } catch(std::bad_alloc) {
        fprintf(stderr, "Not enough memory allocated. Exiting...");
        return 1;
    }
    for (unsigned int i = 0; i < tripsSize; ++i) bestTour[i].score = 0.0;
    double bestTourTotalScore = 0.0;

// === MAIN LOOP ===============================================================
    for (unsigned int iter = 0; iter < iterNumber; ++iter) {
        struct Trip *tour;
        try {
            tour = new struct Trip [tripsSize];
        } catch(std::bad_alloc) {
            fprintf(stderr, "Not enough memory allocated. Exiting...");
            return 1;
        }
        for (unsigned int i = 0; i < tripsSize; ++i) {
            tour[i].totalLength = tripsLength[i];
            tour[i].remainingLength = tripsLength[i];
            tour[i].score = 0.0;
        }

// === TOUR HOTEL SELECTION ====================================================
        tourGreedyRandomizedConstruction(
            tripsSize, tour,
            hotelsSize, vertices,
            distancesMatrix,
            HotelRCLSize);
        if (debug && iter == iterNumber - 1) {
            printf("===========================================\n");
            printf("TOUR HOTEL SELECTION OK!\n");
        }
        else if (debug == 2 && iter % 100 == 0) {
            printf("===========================================\n");
            printf("iter %d: TOUR HOTEL SELECTION OK!\n", iter);
            printTourHotels(tripsSize, tour, distancesMatrix);
        }

// === TOUR HOTELS LOCAL SEARCH ================================================
        tourLocalSearch(
            tripsSize, tour,
            hotelsSize, vertices,
            distancesMatrix); // TODO: implement
        // printTourHotels(tripsSize, tour);

// === TOUR POIS SELECTION =====================================================
        // DEBUG: This part of the code produces a Segmentation fault and I
        //          honestly don't know why it happens.
        tripGreedyRandomizedConstruction(
            tripsSize, tour,
            hotelsSize, poisSize, vertices,
            distancesMatrix,
            PoiRCLSize);

        if (debug && iter == iterNumber - 1) {
            printf("===========================================\n");
            printf("TOUR POI SELECTION OK!\n");
        }
        else if (debug == 2 && iter % 100 == 0) {
            printf("===========================================\n");
            printf("iter %d: TOUR POI SELECTION OK!\n", iter);
            printTours(tripsSize, bestTour);
        }

// === TOUR POIS LOCAL SEARCH ==================================================
        // TODO: implement

// === BEST VS CURRENT TOUR COMPARISON =========================================
        double tourTotalScore = 0.0;
        for (unsigned int i = 0; i < tripsSize; ++i)
            tourTotalScore += tour[i].score;
        if (debug == 2 && iter % 100 == 0) {
            printf("===========================================\n");
            printf("current tour score: %.2f\n", tourTotalScore);
            printf("best tour score:    %.2f\n", bestTourTotalScore);
        }
        if (tourTotalScore > bestTourTotalScore) {
            for (unsigned int i = 0; i < tripsSize; ++i)
                bestTour[i] = tour[i];
            bestTourTotalScore = tourTotalScore;
        }
        delete[] tour;
    }
// === OUTPUT FILE WRITING =====================================================
    // TODO: MAYBE move to it's own function?

    printf("\n%s\n\n", outFile);
    //writeTours(tripsSize, bestTour, outFile);

    if (debug) printf("===========================================\n");
    if (debug) printf("OUTPUT FILE WRITING OK!\n");
// === MEMORY RELEASING ========================================================
    // TODO: move to it's own function maybe?
    delete[] tripsLength;
    delete[] vertices;
    for (unsigned int i = 0; i < hotelsSize + poisSize; ++i) delete[] distancesMatrix[i];
    delete[] distancesMatrix;
    delete[] bestTour;
    if (debug) printf("===========================================\n");
    if (debug) printf("MEMORY RELEASING OK!\n");
    if (debug) printf("===========================================\n");

    return 0;
}

/*
procedure GRASP(max_iter, seed):
    Read_input(); [1]
    best_solution = null;
    for k = 0,..., max_iter do:
        solution = Greedy_randomized_construction(seed);
        if solution is not feasible then: [2]
            solution = Repair(solution); [3]
        end;
        solution = Local_search(solution);
        Update_solution(solution, best_solution);
    end;
    return(best_solution);
end GRASP;

procedure Greedy_randomized_construction(seed):
    solution = null;
    Initialize the set of candidate elements; [4]
    Evaluate the incremental cost of candidate elements; [5]
    while there exists at least one candidate element do:
        Build the restricted_candidate_list(RCL); [6]
        Select an element s from RCL at random;
        solution = solution + s;
        Update the set of candidate elements;
        Reevaluate the incremental costs;
    end;
    return solution;
end Greedy_randomized_construction;

procedure Local_search(solution): [7]
    while solution is not locally optimal do:
        find solution' in the neighborhood of solution with f(solution') < f(solution);
        solution = solution';
    end;
    return solution;
end Local_search;
*/
