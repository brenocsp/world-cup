#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <time.h>
#else
    #include <sys/times.h>
    #include <sys/unistd.h>
#endif

#define WON 3
#define DRAW 1
#define LOST 0
#define MEMORY 0
#define DIRECTORY 1
#define MAX_NAME 50
#define EMPTY "!@#$"
#define MAX_HASH 40000

typedef struct {
    int date,
        homeScore,
        awayScore;
    char homeTeam[MAX_NAME],
         awayTeam[MAX_NAME],
         bothTeams[2 * MAX_NAME],
         tournament[MAX_NAME];
}typeResult;

typedef struct {
    char name[MAX_NAME];
    int points,
        matches,
        victories,
        draws,
        defeats,
        goalsFor,
        goalsAgainst,
        goalsDifference;
    float yield;
}typeTeam;

typedef struct {
    char names[2 * MAX_NAME];
    int numMatches;
}typeMatch;

typedef struct teamTree *typeTeamsTree;
typedef struct teamTree {
    typeTeam team;
    typeTeamsTree left, right;
}typeTeamsLeaf;

typedef struct matchTree *typeMatchesTree;
typedef struct matchTree {
    typeMatch match;
    typeMatchesTree left, right;
}typeMatchesLeaf;

typedef struct teamList *typeTeamsList;
typedef struct teamList {
    typeTeam team;
    typeTeamsList next;
}typeTeamsNode;

typedef struct matchList *typeMatchesList;
typedef struct matchList {
    typeMatch match;
    typeMatchesList next;
}typeMatchesNode;

/* General functions: --------------------------------------------------------------------------------------------- */

float getTime();
void testAllocation (void *pointer, int type);
void createSaidasFile (typeTeam *teams, int numTeams, typeMatch *matches, int numMatches);
void computeScores (typeResult const *match, typeTeam *teamA, typeTeam *teamB);

/* Opening results file: ------------------------------------------------------------------------------------------ */

int numberOfLines (FILE *file);
void skipLines (int numResultsLines, FILE *file);
void defineCopyRange (FILE *file, int *firstLine, int *lastLine, int argc, char const **argv);
typeResult *openAndCopyResults (int *numResultsLines, int const argc, char const **argv);

/* MergeSort for teams: ------------------------------------------------------------------------------------------- */

bool compareTeams (typeTeam *teamA, typeTeam *teamB);
void mergeTeams (typeTeam *teams, int left, int middle, int right);
void sortTeams (typeTeam *teams, int left, int right);
void mergesortTeams (typeTeam *teams, int numTeams);

/* MergeSort for matches: ----------------------------------------------------------------------------------------- */

bool compareMatches (typeMatch *matchA, typeMatch *matchB);
void mergeMatches (typeMatch *matches, int left, int middle, int right);
void sortMatches (typeMatch *matches, int left, int right);
void mergesortMatches (typeMatch *matches, int numMatches);

/* Method 1: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsList (char const *name, typeTeamsList *newNode, int *numTeams);
typeTeamsList searchTeamsList (char const *name, typeTeamsList *head, int *numTeams);
typeTeamsList createTeamsList (typeResult const *results, int const numResultsLines, int *numTeams);
void freeAndConvertTeamsListToArray (typeTeamsList *node, typeTeam *array, int *count);

void insertionMatchesList (char const *names, typeMatchesList *newNode, int *totalMatches);
typeMatchesList searchMatchesList (char const *names, typeMatchesList *head, int *numMatches);
typeMatchesList createMatchesList (typeResult const *results, int const numResultsLines, int *numMatches);
void freeAndConvertMatchesListToArray (typeMatchesList *node, typeMatch *array, int *count);

/* Method 2: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsTree (char const *name, typeTeamsTree *root, int *numTeams);
typeTeamsTree searchTeamsTree (char const *name, typeTeamsTree *root);
typeTeamsTree createTeamsTree (typeResult const *results, int const numResultsLines, int *numTeams);
void freeAndConvertTeamsTreeToArray (typeTeamsTree *node, typeTeam *array, int *count);

void insertionMatchesTree (char const *names, typeMatchesTree *root, int *totalMatches);
typeMatchesTree searchMatchesTree (char const *teams, typeMatchesTree *root);
typeMatchesTree createMatchesTree (typeResult const *results, int const numResultsLines, int *numMatches);
void freeAndConvertMatchesTreeToArray (typeMatchesTree *node, typeMatch *array, int *count);

/* Method 3: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsHashTable (char const *name, typeTeam *hashTable, int const id);
int searchTeamsHashTable (char const *name, typeTeam *hashTable);
typeTeam *createTeamsHashTable (typeResult const *results, int const numResultsLines);

void insertionMatchesHashTable (char const *names, typeMatch *hashTable, int const id);
int searchMatchesHashTable (char const *name, typeMatch *hashTable);
typeMatch *createMatchesHashTable (typeResult const *results, int const numResultsLines);
