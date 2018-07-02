/* -------------------------------------------------------------------------------------------------------------------
    UNIVERSIDADE FEDERAL DE MINAS GERAIS
    Departamento de Ciência da Computação
    DCC004 - Algoritmos e Estruturas de Dados II
    Profs. Cristiano Arbex Valle e Gisele L. Pappa

    TP2 - PREPARAÇÃO PARA A COPA DO MUNDO
    Data de Entrega: 02/07/2018

    Autor: Breno Claudio de Sena Pimenta
    Matrícula: 2017074424
------------------------------------------------------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "world-cup.h"

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

int main (int argc, char const **argv) {

    if (argc < 3) return EXIT_FAILURE;

    int numResultsLines, numTeams, numMatches;
    typeTeam *teamsArray = NULL;
    typeMatch *matchesArray = NULL;
    typeResult *results = openAndCopyResults (&numResultsLines, argc, argv);

    float timeT0 = getTime();
    /* Method 1: */

    if (*argv[2] - 48 == 1) {
        typeTeamsList teamsList = createTeamsList (results, numResultsLines, &numTeams);
        teamsArray = (typeTeam*) calloc (numTeams, sizeof (typeTeam));
        testAllocation (teamsArray, MEMORY);
        int temp = numTeams;
        freeAndConvertTeamsListToArray (&teamsList->next, teamsArray, &temp);
        free (teamsList);

        typeMatchesList matchesList = createMatchesList (results, numResultsLines, &numMatches);
        matchesArray = (typeMatch*) calloc (numMatches, sizeof (typeMatch));
        testAllocation (matchesArray, MEMORY);
        temp = numMatches;
        freeAndConvertMatchesListToArray (&matchesList->next, matchesArray, &temp);
        free (matchesList);
    }

    /* Method 2: */
    else if (*argv[2] - 48 == 2) {
        typeTeamsTree teamsTree = createTeamsTree (results, numResultsLines, &numTeams);
        teamsArray = (typeTeam*) calloc (numTeams, sizeof (typeTeam));
        testAllocation (teamsArray, MEMORY);
        int temp = numTeams;
        freeAndConvertTeamsTreeToArray (&teamsTree, teamsArray, &temp);

        typeMatchesTree matchesTree = createMatchesTree (results, numResultsLines, &numMatches);
        matchesArray = (typeMatch*) calloc (numMatches, sizeof (typeMatch));
        testAllocation (matchesArray, MEMORY);
        temp = numMatches;
        freeAndConvertMatchesTreeToArray (&matchesTree, matchesArray, &temp);
    }

    /* Method 3: */
    else if (*argv[2] - 48 == 3) {
    	numTeams = numResultsLines;
        teamsArray = createTeamsHashTable (results, numResultsLines);

        numMatches = numResultsLines;
        matchesArray = createMatchesHashTable (results, numResultsLines);
    }

    else {
        free (results);
        printf("Invalid method!\n");
        return EXIT_FAILURE;
    }


    float timeT1 = getTime () - timeT0;
    printf("O tempo total em segundos para executar o Metodo %d foi: %.10f\n", *argv[2] - 48, timeT1);

    mergesortTeams (teamsArray, numTeams);
    mergesortMatches (matchesArray, numMatches);
    createSaidasFile (teamsArray, numTeams, matchesArray, numMatches);

    free (results);
    free (teamsArray);
    free (matchesArray);

    return EXIT_SUCCESS;
}
