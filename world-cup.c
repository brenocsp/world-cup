#include "world-cup.h"

/* General Functions: --------------------------------------------------------------------------------------------- */

float getTime() {
    #ifdef _WIN32
        clock_t start = clock();
        return (float)start / CLOCKS_PER_SEC;
    #else
        struct tms tempo;
        clock_t tempotic;
        float cpt = sysconf(_SC_CLK_TCK);
        times(&tempo);
        tempotic = tempo.tms_utime ;
        return ( (float) (tempotic/cpt) );
    #endif
}

void testAllocation (void *pointer, int type){
    if (!pointer) {
        if (type == DIRECTORY) printf("No such file or directory!\n");
        else printf("Not enough memory!\n");
        exit (EXIT_FAILURE);
    }
}

void createSaidasFile (typeTeam *teams, int numTeams, typeMatch *matches, int numMatches) {
    FILE *saidas = fopen ("saidas.txt", "w");
    testAllocation (saidas, DIRECTORY);
    fprintf(saidas, "Ranking,P,J,V,E,D,GM,GS,SG,AP\n");
    for (int i = 0; i < numTeams; i++) {
        if (strcmp (EMPTY, teams[i].name) != 0)
        fprintf(saidas, "%s,%d,%d,%d,%d,%d,%d,%d,%d,%.2f\n", teams[i].name, teams[i].points,
                teams[i].matches, teams[i].victories, teams[i].draws, teams[i].defeats,
                teams[i].goalsFor, teams[i].goalsAgainst, teams[i].goalsDifference, teams[i].yield);
    }
    fprintf (saidas, "\nConfrontos");
    for (int i = 0; i < numMatches; i++) {
        if (strcmp (EMPTY, matches[i].names) != 0 && strcmp (",", matches[i].names) != 0)
        fprintf(saidas, "\n%s,%d", matches[i].names, matches[i].numMatches);
    }
    fclose (saidas);
}

void computeScores (typeResult const *match, typeTeam *teamA, typeTeam *teamB) {
    /* Case 1: team teamA beats team teamB */
    if (match->homeScore > match->awayScore) {
        teamA->points += WON;  teamA->victories += 1;
        teamA->points += LOST; teamB->defeats += 1;
    }

    /* Case 2: team teamB beats team teamA */
    else if (match->homeScore < match->awayScore) {
        teamA->points += LOST; teamA->defeats += 1;
        teamB->points += WON;  teamB->victories += 1;
    }

    /* Case 3: draw */
    else {
        teamA->points += DRAW; teamA->draws += 1;
        teamB->points += DRAW; teamB->draws += 1;
    }

    teamA->matches++;
    teamA->goalsFor += match->homeScore;
    teamA->goalsAgainst += match->awayScore;
    teamA->goalsDifference = teamA->goalsFor - teamA->goalsAgainst;
    teamA->yield = (float) 100 * teamA->points / (3 * teamA->matches);

    teamB->matches++;
    teamB->goalsFor += match->awayScore;
    teamB->goalsAgainst += match->homeScore;
    teamB->goalsDifference = teamB->goalsFor - teamB->goalsAgainst;
    teamB->yield = (float) 100 * teamB->points / (3 * teamB->matches);
}

/* Opening results file: ------------------------------------------------------------------------------------------ */

int numberOfLines (FILE *file) {
    char ch;
    int numResultsLines = 0;
    while (!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n')
            (numResultsLines)++;
    }
    rewind (file);
    return numResultsLines;
}

void skipLines (int numResultsLines, FILE *file) {
    for (int i = 0; i < numResultsLines; i++) {
        fscanf(file,"%*[^\n]\n");
    }
}

void defineCopyRange (FILE *file, int *firstLine, int *lastLine, int argc, char const **argv) {
    int date;
    int numResultsLines = numberOfLines (file);

    if (argc == 3) {
        *firstLine = 1;
        *lastLine = numResultsLines;
    }

    if (argc == 4 || argc == 5) {
        *firstLine = 0;
        *lastLine = numResultsLines;

        while (!feof(file)) {
            skipLines (1, file);
            fscanf (file, "%d", &date);
            if (date > atoi(argv[3]) * 10000) break;
            *firstLine += 1;
        }
        *firstLine += 1;
    }

    if (argc == 5) {
        *lastLine = *firstLine;
        while (!feof(file)) {
            skipLines (1, file);
            fscanf (file, "%d", &date);
            if (date > atoi(argv[4]) * 10000 + 1231) break;
            *lastLine += 1;
        }
        *lastLine += 1;
    }

    rewind (file);
}

typeResult *openAndCopyResults (int *numResultsLines, int const argc, char const **argv) {
    int firstLine, lastLine;
    FILE *resultsFile = fopen(argv[1], "r");
    testAllocation (resultsFile, DIRECTORY);
    defineCopyRange (resultsFile, &firstLine, &lastLine, argc, argv);
    *numResultsLines = lastLine - firstLine;
    typeResult *results = (typeResult*) calloc (*numResultsLines, sizeof(typeResult));
    testAllocation (results, MEMORY);
    skipLines (firstLine, resultsFile);
    for (int i = 0; i < *numResultsLines; i++) {
        fscanf (resultsFile, "%d,%50[^,],%50[^,],%d,%d,%50[^,]",
                &results[i].date, results[i].homeTeam, results[i].awayTeam,
                &results[i].homeScore, &results[i].awayScore, results[i].tournament);
        skipLines (1, resultsFile);
        if (strcmp (results[i].homeTeam, results[i].awayTeam) < 0)
            sprintf (results[i].bothTeams,"%s,%s", results[i].homeTeam, results[i].awayTeam);
        else
            sprintf (results[i].bothTeams,"%s,%s", results[i].awayTeam, results[i].homeTeam);
    }
    fclose (resultsFile);
    return results;
}

/* MergeSort for teams: ------------------------------------------------------------------------------------------- */

bool compareTeams (typeTeam *teamA, typeTeam *teamB) {
    if (teamA->points > teamB->points) return true;
    else if (teamA->points < teamB->points) return false;

    if (teamA->yield > teamB->yield) return true;
    else if (teamA->yield < teamB->yield) return false;

    if (teamA->goalsDifference > teamB->goalsDifference) return true;
    else if (teamA->goalsDifference < teamB->goalsDifference) return false;

    if (teamA->goalsFor > teamB->goalsFor) return true;
    else if (teamA->goalsFor < teamB->goalsFor) return false;

    if (strcmp (teamA->name, teamB->name) < 0) return true;
    else return false;
}

void mergeTeams (typeTeam *teams, int left, int middle, int right){
    int sizeA = middle - left + 1;
    int sizeB = right - middle;
    typeTeam *A = (typeTeam*) calloc (sizeA, sizeof(typeTeam));
    testAllocation (A, MEMORY);
    typeTeam *B = (typeTeam*) calloc (sizeB, sizeof(typeTeam));
    testAllocation (B, MEMORY);
    for (int i = 0; i < sizeA; i++) A[i] = teams[i + left];
    for (int i = 0; i < sizeB; i++) B[i] = teams[i + middle + 1];
    for (int i = 0, j = 0, k = left; k <= right; k++) {
        if (i == sizeA) teams[k] = B[j++];
        else if (j == sizeB) teams[k] = A[i++];
        else if (compareTeams (&A[i], &B[j])) teams[k] = A[i++];
        else teams[k] = B[j++];
    }
    free(A); free(B);
}

void sortTeams (typeTeam *teams, int left, int right) {
    if (left < right) {
        int middle = (left + right) / 2;
        sortTeams (teams, left, middle);
        sortTeams (teams, middle + 1, right);
        mergeTeams (teams, left, middle, right);
    }
}

void mergesortTeams (typeTeam *teams, int numTeams) {
    sortTeams (teams, 0, numTeams-1);
}

/* MergeSort for matches: ----------------------------------------------------------------------------------------- */

bool compareMatches (typeMatch *matchA, typeMatch *matchB) {
    if (matchA->numMatches > matchB->numMatches) return true;
    else if (matchA->numMatches < matchB->numMatches) return false;

    if (strcmp (matchA->names, matchB->names) < 0) return true;
    else return false;
}

void mergeMatches (typeMatch *matches, int left, int middle, int right){
    int sizeA = middle - left + 1;
    int sizeB = right - middle;
    typeMatch *A = (typeMatch*) calloc (sizeA, sizeof(typeMatch));
    testAllocation (A, MEMORY);
    typeMatch *B = (typeMatch*) calloc (sizeB, sizeof(typeMatch));
    testAllocation (B, MEMORY);
    for (int i = 0; i < sizeA; i++) A[i] = matches[i + left];
    for (int i = 0; i < sizeB; i++) B[i] = matches[i + middle + 1];
    for (int i = 0, j = 0, k = left; k <= right; k++) {
        if (i == sizeA) matches[k] = B[j++];
        else if (j == sizeB) matches[k] = A[i++];
        else if (compareMatches (&A[i], &B[j])) matches[k] = A[i++];
        else matches[k] = B[j++];
    }
    free(A); free(B);
}

void sortMatches (typeMatch *matches, int left, int right) {
    if (left < right) {
        int middle = (left + right) / 2;
        sortMatches (matches, left, middle);
        sortMatches (matches, middle + 1, right);
        mergeMatches (matches, left, middle, right);
    }
}
void mergesortMatches (typeMatch *matches, int numMatches) {
    sortMatches (matches, 0, numMatches-1);
}

/* Method 1: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsList (char const *name, typeTeamsList *newNode, int *numTeams) {
    *newNode = (typeTeamsList) calloc (1, sizeof(typeTeamsNode));
    testAllocation (newNode, MEMORY);
    (*newNode)->next = NULL;
    (*numTeams) += 1;
    strcpy ((*newNode)->team.name, name);
}

typeTeamsList searchTeamsList (char const *name, typeTeamsList *head, int *numTeams) {
    typeTeamsList *aux = head;
    if ((*aux)->next == NULL) {
        insertionTeamsList (name, &((*aux)->next), numTeams);
        return (*aux)->next;
    }
    while (strcmp (name, (*aux)->team.name) != 0 && (*aux)->next != NULL)
        aux = &((*aux)->next);
    if (strcmp (name, (*aux)->team.name) != 0) {
        insertionTeamsList (name, &((*aux)->next), numTeams);
        aux = &((*aux)->next);
    }
    return *aux;
}

typeTeamsList createTeamsList (typeResult const *results, int const numResultsLines, int *numTeams) {
    *numTeams = 0;
    typeTeamsList head = (typeTeamsList) calloc (1, sizeof (typeTeamsNode));
    testAllocation (head, MEMORY);
    head->next = NULL;
    for (int i = 0; i < numResultsLines; i++) {
        if (strcmp("FIFA World Cup", results[i].tournament) == 0) {
            typeTeamsList teamA = searchTeamsList (results[i].homeTeam, &head, numTeams);
            typeTeamsList teamB = searchTeamsList (results[i].awayTeam, &head, numTeams);
            computeScores (&results[i], &teamA->team, &teamB->team);
        }
    }
    return head;
}

void freeAndConvertTeamsListToArray (typeTeamsList *node, typeTeam *array, int *count) {
    if (*node == NULL) return;
    (*count)--;
    array[*count] = (*node)->team;
    freeAndConvertTeamsListToArray (&(*node)->next, array, count);
    free (*node);
}

void insertionMatchesList (char const *names, typeMatchesList *newNode, int *numMatches) {
    *newNode = (typeMatchesList) calloc (1, sizeof(typeMatchesNode));
    testAllocation (newNode, MEMORY);
    (*newNode)->next = NULL;
    (*numMatches) += 1;
    strcpy ((*newNode)->match.names, names);
}

typeMatchesList searchMatchesList (char const *names, typeMatchesList *head, int *numMatches) {
    typeMatchesList *aux = head;
    if ((*aux)->next == NULL) {
        insertionMatchesList (names, &((*aux)->next), numMatches);
        return (*aux)->next;
    }
    while (strcmp (names, (*aux)->match.names) != 0 && (*aux)->next != NULL)
        aux = &((*aux)->next);
    if (strcmp (names, (*aux)->match.names) != 0) {
        insertionMatchesList (names, &((*aux)->next), numMatches);
        aux = &((*aux)->next);
    }
    return *aux;
}

typeMatchesList createMatchesList (typeResult const *results, int const numResultsLines, int *numMatches) {
    *numMatches = 0;
    typeMatchesList head = (typeMatchesList) calloc (1, sizeof (typeMatchesNode));
    testAllocation (head, MEMORY);
    head->next = NULL;
    // for (int i = 0; i < numResultsLines; i++)
    //     insertionMatchesList (results[i].bothTeams, &head, numMatches);
    for (int i = 0; i < numResultsLines; i++) {
        typeMatchesList A = searchMatchesList (results[i].bothTeams, &head, numMatches);
        A->match.numMatches += 1;
    }
    return head;
}

void freeAndConvertMatchesListToArray (typeMatchesList *node, typeMatch *array, int *count) {
    if (*node == NULL) return;
    (*count)--;
    array[*count] = (*node)->match;
    freeAndConvertMatchesListToArray (&(*node)->next, array, count);
    free (*node);
}

/* Method 2: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsTree (char const *name, typeTeamsTree *root, int *numTeams) {
    if (*root == NULL) {
        *root = (typeTeamsLeaf*) calloc (1, sizeof(typeTeamsLeaf));
        testAllocation (root, MEMORY);
        (*root)->left = NULL;
        (*root)->right = NULL;
        *numTeams += 1;
        strcpy ((*root)->team.name, name);
    }
    else if (strcmp((*root)->team.name, name) > 0)
        insertionTeamsTree (name, &((*root)->left), numTeams);
    else if (strcmp((*root)->team.name, name) < 0)
        insertionTeamsTree (name, &((*root)->right), numTeams);
}

typeTeamsTree searchTeamsTree (char const *name, typeTeamsTree *root) {
    if (*root == NULL || strcmp ((*root)->team.name, name) == 0)
        return *root;
    else if (strcmp((*root)->team.name, name) > 0)
        return searchTeamsTree (name, &(*root)->left);
    else
        return searchTeamsTree (name, &(*root)->right);
}

typeTeamsTree createTeamsTree (typeResult const *results, int const numResultsLines, int *numTeams) {
    *numTeams = 0;
    typeTeamsTree root = NULL;
    for (int i = 0; i < numResultsLines; i++) {
         if (strcmp("FIFA World Cup", results[i].tournament) == 0) {
             insertionTeamsTree (results[i].homeTeam, &root, numTeams);
             insertionTeamsTree (results[i].awayTeam, &root, numTeams);
         }
    }
    for (int i = 0; i < numResultsLines; i++) {
        if (strcmp("FIFA World Cup", results[i].tournament) == 0) {
            typeTeamsTree teamA = searchTeamsTree (results[i].homeTeam, &root);
            typeTeamsTree teamB = searchTeamsTree (results[i].awayTeam, &root);
            computeScores (&results[i], &teamA->team, &teamB->team);
        }
    }
    return root;
}

void freeAndConvertTeamsTreeToArray (typeTeamsTree *node, typeTeam *array, int *count) {
    if (*node == NULL) return;
    (*count)--;
    array[*count] = (*node)->team;
    freeAndConvertTeamsTreeToArray (&(*node)->right, array, count);
    freeAndConvertTeamsTreeToArray (&(*node)->left, array, count);
    free (*node);
}

void insertionMatchesTree (char const *names, typeMatchesTree *root, int *totalMatches) {
    if (*root == NULL) {
        *root = (typeMatchesLeaf*) calloc (1, sizeof(typeMatchesLeaf));
        testAllocation (root, MEMORY);
        (*root)->left = NULL;
        (*root)->right = NULL;
        *totalMatches += 1;
        strcpy ((*root)->match.names, names);
    }
    else if (strcmp (names, (*root)->match.names) < 0)
        insertionMatchesTree (names, &((*root)->left), totalMatches);
    else if (strcmp (names, (*root)->match.names) > 0)
        insertionMatchesTree (names, &((*root)->right), totalMatches);
}

typeMatchesTree searchMatchesTree (char const *teams, typeMatchesTree *root) {
    if (*root == NULL || strcmp ((*root)->match.names, teams) == 0)
        return *root;
    else if (strcmp (teams, (*root)->match.names) < 0)
        return searchMatchesTree (teams, &(*root)->left);
    else
        return searchMatchesTree (teams, &(*root)->right);
}

typeMatchesTree createMatchesTree (typeResult const *results, int const numResultsLines, int *numMatches) {
    *numMatches = 0;
    typeMatchesTree root = NULL;
    for (int i = 0; i < numResultsLines; i++)
        insertionMatchesTree (results[i].bothTeams, &root, numMatches);
    for (int i = 0; i < numResultsLines; i++) {
        typeMatchesTree A = searchMatchesTree (results[i].bothTeams, &root);
        A->match.numMatches += 1;
    }
    return root;
}

void freeAndConvertMatchesTreeToArray (typeMatchesTree *node, typeMatch *array, int *count) {
    if (*node == NULL) return;
    (*count)--;
    array[*count] = (*node)->match;
    freeAndConvertMatchesTreeToArray (&(*node)->right, array, count);
    freeAndConvertMatchesTreeToArray (&(*node)->left, array, count);
    free (*node);
}

/* Method 3: ------------------------------------------------------------------------------------------------------ */

void insertionTeamsHashTable (char const *name, typeTeam *hashTable, int const id) {
    strcpy (hashTable[id].name, name);
    hashTable[id].points = 0;
    hashTable[id].matches = 0;
    hashTable[id].victories = 0;
    hashTable[id].draws = 0;
    hashTable[id].defeats = 0;
    hashTable[id].goalsFor = 0;
    hashTable[id].goalsAgainst = 0;
    hashTable[id].goalsDifference = 0;
    hashTable[id].yield = 0;
}

int searchTeamsHashTable (char const *name, typeTeam *hashTable) {
    int id = 1000 * (strlen (name) + name[0] + name[1] + name[2]) % MAX_HASH;
    while (strcmp(EMPTY, hashTable[id].name) != 0 && strcmp(name, hashTable[id].name) != 0) id++;
    if (strcmp(EMPTY, hashTable[id].name) == 0) insertionTeamsHashTable (name, hashTable, id);
    return id;
}

typeTeam *createTeamsHashTable (typeResult const *results, int const numResultsLines) {
    typeTeam *hashTable = (typeTeam*) calloc (MAX_HASH, sizeof(typeTeam));
    testAllocation (hashTable, MEMORY);
    for (int i = 0; i < MAX_HASH; i++) strcpy (hashTable[i].name, EMPTY);
    for (int i = 0; i < numResultsLines; i++) {
        if (strcmp ("FIFA World Cup", results[i].tournament) == 0) {
            int teamA = searchTeamsHashTable (results[i].homeTeam, hashTable);
            int teamB = searchTeamsHashTable (results[i].awayTeam, hashTable);
            computeScores (&results[i], &hashTable[teamA], &hashTable[teamB]);
        }
    }
    return hashTable;
}

void insertionMatchesHashTable (char const *names, typeMatch *hashTable, int const id) {
    strcpy (hashTable[id].names, names);
    hashTable[id].numMatches = 0;
}

int searchMatchesHashTable (char const *name, typeMatch *hashTable) {
    int id = 1000 * (strlen (name) + name[0] + name[1] + name[2]) % MAX_HASH;
    while (strcmp(EMPTY, hashTable[id].names) != 0 && strcmp(name, hashTable[id].names) != 0) id++;
    if (strcmp(EMPTY, hashTable[id].names) == 0) insertionMatchesHashTable (name, hashTable, id);
    return id;
}

typeMatch *createMatchesHashTable (typeResult const *results, int const numResultsLines) {
    typeMatch *hashTable = (typeMatch*) calloc (MAX_HASH, sizeof(typeMatch));
    testAllocation (hashTable, MEMORY);
    for (int i = 0; i < MAX_HASH; i++) strcpy (hashTable[i].names, EMPTY);
    for (int i = 0; i < numResultsLines; i++) {
        int match = searchMatchesHashTable (results[i].bothTeams, hashTable);
        hashTable[match].numMatches += 1;
    }
    return hashTable;
}
