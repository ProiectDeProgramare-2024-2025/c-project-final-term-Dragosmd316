#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#define MAX_QUESTIONS 50
#define MAX_PLAYERS 100
#define MAX_UNFINISHED_GAMES 100

#ifdef _WIN32
    #define CLEAR_COMMAND "cls"
#else
    #define CLEAR_COMMAND "clear"
#endif

void clearScreen() {
    system(CLEAR_COMMAND);
}

typedef struct {
    char name[50];
    int score;
    int questionsAnswered;
    int questionsAsked[MAX_QUESTIONS];
} UnfinishedGame;

typedef struct {
    char name[50];
    int score;
} Player;

typedef struct {
    int difficulty;
    char question[256];
    char options[4][100];
    char correct_answer;
} Question;

Question questions[MAX_QUESTIONS];
Player players[MAX_PLAYERS];
UnfinishedGame unfinishedGames[MAX_UNFINISHED_GAMES];
int num_players = 0;
int num_questions = 0;
int num_unfinished_games = 0;


void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void waitForEnter() {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n');
}

void loadQuestions() {
    FILE *file = fopen("intrebari.txt", "r");
    if (file == NULL) {
        printf("Error opening questions file!\n");
        exit(1);
    }

    while (fscanf(file, "%d", &questions[num_questions].difficulty) != EOF) {
        fgetc(file);
        fgets(questions[num_questions].question, 256, file);
        questions[num_questions].question[strcspn(questions[num_questions].question, "\n")] = '\0';
        for (int i = 0; i < 4; i++) {
            fgets(questions[num_questions].options[i], 100, file);
            questions[num_questions].options[i][strcspn(questions[num_questions].options[i], "\n")] = '\0';
        }
        fscanf(file, " %c", &questions[num_questions].correct_answer);
        fgetc(file);
        num_questions++;
    }
    fclose(file);
}

void loadLeaderboard() {
    FILE *file = fopen("leaderboard.txt", "r");
    if (file != NULL) {
        while (fscanf(file, "%s %d", players[num_players].name, &players[num_players].score) != EOF) {
            num_players++;
        }
        fclose(file);
    }
}

void saveLeaderboard() {
    FILE *file = fopen("leaderboard.txt", "w");
    if (file == NULL) {
        printf("Error opening leaderboard file!\n");
        return;
    }

    for (int i = 0; i < num_players; i++) {
        for (int j = i + 1; j < num_players; j++) {
            if (players[i].score < players[j].score) {
                Player temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }

    for (int i = 0; i < num_players; i++) {
        fprintf(file, "%s %d\n", players[i].name, players[i].score);
    }
    fclose(file);
}

void saveUnfinishedGames() {
    FILE *file = fopen("unfinished_games.txt", "w");
    if (file == NULL) {
        printf("Error saving unfinished games!\n");
        return;
    }

    for (int i = 0; i < num_unfinished_games; i++) {
        if (unfinishedGames[i].questionsAnswered == 10) {
            continue;
        }

        int isDuplicate = 0;
        for (int j = i + 1; j < num_unfinished_games; j++) {
            if (strcmp(unfinishedGames[i].name, unfinishedGames[j].name) == 0) {
                isDuplicate = 1;
                break;
            }
        }

        if (!isDuplicate) {
            fprintf(file, "%s %d %d ", unfinishedGames[i].name, unfinishedGames[i].score, unfinishedGames[i].questionsAnswered);
            for (int j = 0; j < num_questions; j++) {
                fprintf(file, "%d ", unfinishedGames[i].questionsAsked[j]);
            }
            fprintf(file, "\n");
        }
    }

    fclose(file);
}

void loadUnfinishedGames() {
    FILE *file = fopen("unfinished_games.txt", "r");
    if (file != NULL) {
        num_unfinished_games = 0;

        while (fscanf(file, "%s %d %d", unfinishedGames[num_unfinished_games].name,
                      &unfinishedGames[num_unfinished_games].score,
                      &unfinishedGames[num_unfinished_games].questionsAnswered) != EOF) {
            for (int i = 0; i < num_questions; i++) {
                fscanf(file, "%d", &unfinishedGames[num_unfinished_games].questionsAsked[i]);
            }

            if (unfinishedGames[num_unfinished_games].questionsAnswered == 10) {
                continue;
            }

            num_unfinished_games++;
        }

        fclose(file);

        for (int i = 0; i < num_unfinished_games; i++) {
            int foundInLeaderboard = 0;
            for (int j = 0; j < num_players; j++) {
                if (strcmp(unfinishedGames[i].name, players[j].name) == 0) {
                    foundInLeaderboard = 1;
                    break;
                }
            }
            if (foundInLeaderboard) {
                for (int k = i; k < num_unfinished_games - 1; k++) {
                    unfinishedGames[k] = unfinishedGames[k + 1];
                }
                num_unfinished_games--;
                i--;
            }
        }
    }
}


void askQuestion(Question *q, int *score) {
    clearScreen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    printf("\n%s\n", q->question);

    for (int i = 0; i < 4; i++) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("%c", 'A' + i);
        SetConsoleTextAttribute(hConsole, 7);
        printf(") %s\n", q->options[i] + 3);
    }
    SetConsoleTextAttribute(hConsole, 7);

    printf("Enter your answer (");
    const char letters[] = {'A', 'B', 'C', 'D'};
    for (int i = 0; i < 4; i++) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        printf("%c", letters[i]);
        SetConsoleTextAttribute(hConsole, 7);
        if (i < 3) printf(", ");
        else printf("): ");
    }

    char answer;
    if (scanf(" %c", &answer) != 1) {
        printf("Invalid input! Skipping question.\n");
        clearInputBuffer();
        return;
    }

    clearInputBuffer();

    if (tolower(answer) == tolower(q->correct_answer)) {
        *score += q->difficulty;

        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("Correct!");
        SetConsoleTextAttribute(hConsole, 7);
        printf(" You've earned %d points.\n", q->difficulty);

    } else {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("Incorrect.");
        SetConsoleTextAttribute(hConsole, 7);
        printf(" The correct answer was ");

        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("%c", toupper(q->correct_answer));
        SetConsoleTextAttribute(hConsole, 7);
        printf(".\n");
    }

    waitForEnter();
}



void startUnfinishedGame(int gameIndex) {
    UnfinishedGame *game = &unfinishedGames[gameIndex];

    for (int i = game->questionsAnswered; i < 10; i++) {
        int qIndex;
        do {
            qIndex = rand() % num_questions;
        } while (game->questionsAsked[qIndex]);

        game->questionsAsked[qIndex] = 1;
        askQuestion(&questions[qIndex], &game->score);

        game->questionsAnswered++;

        if (game->questionsAnswered < 10) {
            saveUnfinishedGames();
        }
    }

    if (game->questionsAnswered == 10) {
        printf("\nGame Over!\n");
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("Your final score: %d\n", game->score);
        SetConsoleTextAttribute(hConsole, 7);

        strcpy(players[num_players].name, game->name);
        players[num_players++].score = game->score;
        saveLeaderboard();

        for (int j = gameIndex; j < num_unfinished_games - 1; j++) {
            unfinishedGames[j] = unfinishedGames[j + 1];
        }
        num_unfinished_games--;
        saveUnfinishedGames();

        waitForEnter();
    }
}



void startNewGame() {
    UnfinishedGame newGame;
    memset(&newGame, 0, sizeof(UnfinishedGame));

    printf("Enter your name: ");
    scanf("%s", newGame.name);
    clearInputBuffer();

    for (int i = 0; i < 10; i++) {
        int qIndex;
        do {
            qIndex = rand() % num_questions;
        } while (newGame.questionsAsked[qIndex]);

        newGame.questionsAsked[qIndex] = 1;
        askQuestion(&questions[qIndex], &newGame.score);

        newGame.questionsAnswered++;

        if (newGame.questionsAnswered < 10) {
            unfinishedGames[num_unfinished_games] = newGame;
            num_unfinished_games++;
            saveUnfinishedGames();
        }
    }

    printf("\n");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("Game Over!");
    SetConsoleTextAttribute(hConsole, 7);
    printf(" You scored ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("%d points", newGame.score);
    SetConsoleTextAttribute(hConsole, 7);
    printf(".\n");

    waitForEnter();

    strcpy(players[num_players].name, newGame.name);
    players[num_players++].score = newGame.score;

    saveLeaderboard();
}


void showUnfinishedGames() {
    clearScreen();
    printf("\nUnfinished Games:\n");
    for (int i = 0; i < num_unfinished_games; i++) {
        printf("%d. %s - Score: %d, Questions Answered: %d\n",
               i + 1, unfinishedGames[i].name, unfinishedGames[i].score, unfinishedGames[i].questionsAnswered);
    }
    printf("\n");
}

void startGame() {
    loadUnfinishedGames();

    if (num_unfinished_games > 0) {
        printf("Do you want to continue an unfinished game? (Y/N): ");
        char choice;
        scanf(" %c", &choice);
        clearInputBuffer();

        if (tolower(choice) == 'y') {
            showUnfinishedGames();
            printf("Choose a game to continue: ");
            int gameIndex;
            if (scanf("%d", &gameIndex) != 1 || gameIndex < 1 || gameIndex > num_unfinished_games) {
                printf("Invalid choice! Starting a new game.\n");
                clearInputBuffer();
                startNewGame();
            } else {
                clearInputBuffer();
                startUnfinishedGame(gameIndex - 1);
            }
            return;
        }
    }

    startNewGame();
}

void showMenu() {
    clearScreen();
    printf("\nTrivia Game Menu:\n");
    printf("1. Start Game\n");
    printf("2. View Leaderboard\n");
    printf("3. Exit\n");
    printf("Enter your choice (1,2,3): ");
}

void showLeaderboard() {
    clearScreen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
printf("\n");
SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
printf("Leaderboard:\n");
SetConsoleTextAttribute(hConsole, 7);

    for (int i = 0; i < num_players; i++) {
        printf("%s: %d points\n", players[i].name, players[i].score);
    }
    waitForEnter();
}

int main() {
    srand(time(NULL));
    loadQuestions();
    loadLeaderboard();

    int choice;
    do {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a valid option.\n");
            clearInputBuffer();
            continue;
        }

        clearInputBuffer();
        switch (choice) {
            case 1:
                startGame();
                break;
            case 2:
                showLeaderboard();
                break;
            case 3:
                printf("Exiting the game. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 3);

    return 0;
}
