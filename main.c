#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MEMORY 0
#define DIRECTORY 1
#define MAX_NAME 50
#define MAX_SIZE 260

typedef struct {
    int year,
        month,
        day,
        home_score,
        away_score;
    char home_team[MAX_NAME],
         away_team[MAX_NAME],
         tournament[MAX_NAME],
         city[MAX_NAME],
         country[MAX_NAME],
         neutral[MAX_NAME];
}type_results;

typedef struct {
    char country[MAX_NAME];
    int points,
        games,
        victories,
        draws,
        defeats,
        goals_for,
        goals_against,
        goals_difference;
    float yield;
}team;

void test_allocation (void *pointer, int type){
    if (!pointer) {
        if (type == DIRECTORY) printf("No such file or directory!\n");
        else printf("Not enough memory!\n");
        exit (EXIT_FAILURE);
    }
}

type_results *open_and_copy_results (int *num_lines, char const *file_name) {
    FILE *results_file = fopen(file_name, "r");
    test_allocation (results_file, DIRECTORY);

    // Count how many lines the file has:
    char ch;
    *num_lines = -1; // desconsider results.csv header
    while (!feof(results_file)) {
        ch = fgetc(results_file);
        if(ch == '\n') (*num_lines)++;
    }
    rewind (results_file); // go back to the beginning of the file
    while (ch != '\n') ch = fgetc(results_file); // skip the header
    type_results *results = (type_results*) calloc (*num_lines, sizeof(type_results));
    test_allocation (results, MEMORY);
    for (int i = 0; i < *num_lines; i++) {
        fscanf (results_file, "%d-%d-%d,%50[^,],%50[^,],%d,%d,%50[^,],%50[^,],%50[^,],%50[^\n]",
                &results[i].year, &results[i].month, &results[i].day, results[i].home_team, results[i].away_team, &results[i].home_score,
                &results[i].away_score, results[i].tournament, results[i].city, results[i].country, results[i].neutral);
    }
    fclose (results_file);
    return (results);
}

int main(int argc, char const **argv) {
    int num_lines;
    type_results *results = open_and_copy_results (&num_lines, argv[1]);
    // test only:
    // for (int i = 0; i < num_lines; i++) {
    //     printf ("%d-%d-%d,%s,%s,%d,%d,%s,%s,%s,%s\n", results[i].year, results[i].month,
    //             results[i].day, results[i].home_team, results[i].away_team, results[i].home_score,
    //             results[i].away_score, results[i].tournament, results[i].city, results[i].country,
    //             results[i].neutral);
    // }

    free (results);
    return EXIT_SUCCESS;
}
