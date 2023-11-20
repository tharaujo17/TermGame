#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

// Códigos de cores ANSI
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_GREY     "\x1b[90m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_RESET    "\x1b[0m"

// Lista encadeada para o ranking
typedef struct RankingNode {
    char playerName[20];
    int attempts;
    struct RankingNode *next;
} RankingNode;

// Lista encadeada para as tentativas
typedef struct AttemptNode {
    char attempt[6];
    char feedback[6];
    struct AttemptNode *next;
} AttemptNode;

typedef struct RankingEntry{
    char playerName[20];
    int attempts;
} RankingEntry;

AttemptNode *createNode(char *attempt);
void playGame();
bool isValidAttempt(char *attempt);
void appendAttempt(AttemptNode **head, char *attempt);
void toUpperCase(char *str);
void generateFeedback(char *attempt, char *secretWord, AttemptNode *head);
void loadRanking(RankingNode **ranking);
void updateRanking(RankingNode **ranking, char *playerName, int attempts);
void printRanking(RankingNode *ranking);
void printColoredLetter(char letter, char colorCode);
void printAttempts(AttemptNode *head);
void insertNodeInSortedOrder(RankingNode **head, RankingNode *newNode);
char* chooseRandomWordFromFile();
// Crie uma lista encadeada para o ranking
RankingNode *rankingList = NULL;
FILE *palavras;

int main(char *argv[]) {
    loadRanking(&rankingList);

    int menuOption;
    do {
        printf(ANSI_COLOR_CYAN "-------------------\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "|     MENU DO TERM    |\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "-------------------\n" ANSI_COLOR_RESET);
        printf("1. Jogar\n");
        printf("2. Ver Ranking\n");
        printf("0. Sair\n");
        printf(ANSI_COLOR_CYAN "Escolha uma alternativa: " ANSI_COLOR_RESET);
        scanf("%d", &menuOption);

        switch(menuOption) {
            case 1:
                playGame();
                break;
            case 2:
                if (rankingList == NULL) {
                    printf("%sO ranking esta vazio! Jogue para adicionar uma nova pontuaçao.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
                } else {
                    printRanking(rankingList);
                }
                break;
            case 0:
                printf("%sObrigado por jogar! Ate a proxima.%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
                break;
            default:
                printf("%sOpçao invalida. Por favor, use uma alternativa valida.%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
        }
    } while(menuOption != 0);

    // Liberar a memória da lista encadeada do ranking
    while (rankingList != NULL) {
        RankingNode *temp = rankingList;
        rankingList = rankingList->next;
        free(temp);
    }
    return 0;
}

void playGame() {
    char* secretWord = chooseRandomWordFromFile();
    AttemptNode *attemptsList = NULL;
    int attempts = 1;

    printf("%sVoce tem 6 tentativas para adivinhar uma palavra de 5 letras.%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);

        while (attempts <= 6) {
        char userAttempt[10];  // Buffer maior para validar o input 
        printf("Tentativa %d: ", attempts);
        scanf("%9s", userAttempt);  // Lê até 9 caracteres para evitar overflow

        toUpperCase(userAttempt);
        if (strlen(userAttempt) > 5) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            printf("Por favor, digite uma palavra com exatamente 5 letras.\n");
            continue;
        }

        if (!isValidAttempt(userAttempt)) {
            printf("Por favor, digite uma palavra com exatamente 5 letras.\n");
            continue;
        }

        appendAttempt(&attemptsList, userAttempt);
        generateFeedback(userAttempt, secretWord, attemptsList);

        printAttempts(attemptsList);

        if (strcmp(userAttempt, secretWord) == 0) {
            printf("Parabens! Voce adivinhou a palavra!\n");
            break;
        }

        attempts++;
    }

    if (attempts == 6) {
        printf("Fim do jogo! A palavra era: %s\n", secretWord);
    }

    printf("Digite seu nome: ");
    char playerName[20];
    scanf("%19s", playerName);
    updateRanking(&rankingList, playerName, attempts);

    printf("Deseja voltar ao menu principal? (s/n) ");
    char answer;
    scanf(" %c", &answer);
    if (answer == 's' || answer == 'S') {
        // Limpa a memória das tentativas antes de voltar ao menu
        while (attemptsList != NULL) {
            AttemptNode *temp = attemptsList;
            attemptsList = attemptsList->next;
            free(temp);
        }
        // Retorna ao menu principal
    } else {
        exit(0);  // Sai do jogo completamente
    }
}

// Função para carregar o ranking do arquivo
void loadRanking(RankingNode **ranking) {
    FILE *file = fopen("ranking.txt", "r+");
    if (file == NULL) {
        // Se o arquivo não existe, não há entradas no ranking
        return;
    }

    char playerName[20];
    int attempts;

    while (fscanf(file, "%s %d", playerName, &attempts) == 2) {
        RankingNode *newNode = (RankingNode *)malloc(sizeof(RankingNode));
        if (newNode == NULL) {
            printf("Erro ao alocar memória para o ranking.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        strcpy(newNode->playerName, playerName);
        newNode->attempts = attempts;
        newNode->next = *ranking;

        *ranking = newNode;
    }

    fclose(file);
}

// Função para atualizar o ranking
void updateRanking(RankingNode **ranking, char *playerName, int attempts) {
    // Verifica se o jogador já existe no ranking
    RankingNode *current = *ranking;
    while (current != NULL) {
        if (strcmp(current->playerName, playerName) == 0) {
            // Atualiza as tentativas se o jogador já existe
            if (attempts < current->attempts) {
                current->attempts = attempts;
            }
            break;  // Não precisa continuar procurando
        }
        current = current->next;
    }

    // Se o jogador não existe, adiciona uma nova entrada
    if (current == NULL) {
        RankingNode *newNode = (RankingNode *)malloc(sizeof(RankingNode));
        if (newNode == NULL) {
            printf("Erro ao alocar memória para o ranking.\n");
            exit(EXIT_FAILURE);
        }

        strcpy(newNode->playerName, playerName);
        newNode->attempts = attempts;
        newNode->next = *ranking;

        *ranking = newNode;
    }

    // Reordena o ranking usando Insertion Sort
    RankingNode *sorted = NULL;
    current = *ranking;
    while (current != NULL) {
        RankingNode *next = current->next;
        insertNodeInSortedOrder(&sorted, current);
        current = next;
    }

    // Atualiza o ranking original com a ordem correta
    *ranking = sorted;

    // Salva o ranking atualizado no arquivo
    FILE *file = fopen("ranking.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de ranking.\n");
        exit(EXIT_FAILURE);
    }

    // Escreve as entradas atualizadas no arquivo
    current = *ranking;
    while (current != NULL) {
        fprintf(file, "%s %d\n", current->playerName, current->attempts);
        current = current->next;
    }

    fclose(file);
}

// Função para inserir um nó ordenadamente em uma lista
void insertNodeInSortedOrder(RankingNode **head, RankingNode *newNode) {
    if (*head == NULL || (*head)->attempts >= newNode->attempts) {
        newNode->next = *head;
        *head = newNode;
    } else {
        RankingNode *current = *head;
        while (current->next != NULL && current->next->attempts < newNode->attempts) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// Função para imprimir o ranking
void printRanking(RankingNode *ranking) {
    printf("\nRanking:\n");
    while (ranking != NULL) {
        printf("%s - %d tentativas\n", ranking->playerName, ranking->attempts);
        ranking = ranking->next;
    }
}

// Função para adicionar tentativa na lista encadeada
void appendAttempt(AttemptNode **head, char *attempt) {
    AttemptNode *newNode = (AttemptNode *)malloc(sizeof(AttemptNode));
    if (newNode == NULL) {
        printf("Erro ao alocar memória para a tentativa.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->attempt, attempt);
    memset(newNode->feedback, ' ', 5);
    newNode->feedback[5] = '\0';
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        AttemptNode *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

// Função para imprimir a lista de tentativas e feedbacks
void printAttempts(AttemptNode *head) {
    AttemptNode *temp = head;
    if(temp->next != NULL){
        while(temp->next != NULL){
            temp = temp->next;
        }
    }
    printf("Tentativa: %s - Feedback: ", temp->attempt);
    for (int i = 0; i < 5; i++) {
        printColoredLetter(temp->attempt[i], temp->feedback[i]);
    }
    printf("\n");
    
}

// Criar novo nó
AttemptNode* createNode(char *attempt) {
    AttemptNode *newNode = (AttemptNode*) malloc(sizeof(AttemptNode));
    if (newNode == NULL) {
        printf("Erro.\n");
        exit(1);
    }
    strcpy(newNode->attempt, attempt);
    memset(newNode->feedback, ' ', 5);
    newNode->feedback[5] = '\0';
    newNode->next = NULL;
    return newNode;
}

// Adicionar no final da lista
void appendNode(AttemptNode **head, char *attempt) {
    AttemptNode *newNode = createNode(attempt);
    if (*head == NULL) {
        *head = newNode;
    } else {
        AttemptNode *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

void toUpperCase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}
// Gerar feedback baseado na tentativa e palavra secreta
void generateFeedback(char *attempt, char *secret, AttemptNode *head) {
    AttemptNode *current = head;
    while (current != NULL) {
        for (int i = 0; i < 5; i++) {
            if (attempt[i] == secret[i]) {
                current->feedback[i] = 'G';
            } else {
                bool found = false;
                for (int j = 0; j < 5; j++) {
                    if (i != j && attempt[i] == secret[j]) {
                        found = true;
                        break;
                    }
                }
                current->feedback[i] = found ? 'Y' : 'C';
            }
        }
        current = current->next;  // Avançar para o próximo nó
    }
    
}


// Printar a letra com a cor certa
void printColoredLetter(char letter, char colorCode) {
    switch (colorCode) {
        case 'G':
            printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, letter);
            break;
        case 'Y':
            printf(ANSI_COLOR_YELLOW "%c" ANSI_COLOR_RESET, letter);
            break;
        case 'C':
            printf(ANSI_COLOR_GREY "%c" ANSI_COLOR_RESET, letter);
            break;
    }
}

// Printar a lista de tentativas e feedbacks


// Verificar se o input tem 5 caracteres.
bool isValidAttempt(char *attempt) {
    int length = strlen(attempt);
    return length == 5;
}


char* chooseRandomWordFromFile() {
    FILE* file = fopen("palavras.txt", "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de palavras.\n");
        exit(1);
    }

    // Conte o número de linhas no arquivo
    int numLines = 0;
    char line[100];  // Assuma que nenhuma linha terá mais de 100 caracteres
    while (fgets(line, sizeof(line), file) != NULL) {
        numLines++;
    }

    // Gere uma semente para a função rand() com base no tempo atual
    srand((unsigned int)time(NULL));

    // Escolha um número aleatório entre 0 e numLines-1
    int randomIndex = rand() % numLines;

    // Reposicione o ponteiro do arquivo para o início
    rewind(file);

    // Pule as linhas até chegar à linha escolhida aleatoriamente
    for (int i = 0; i < randomIndex; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            printf("Erro ao ler o arquivo de palavras.\n");
            fclose(file);
            exit(1);
        }
    }

    // Remova a quebra de linha da palavra lida
    line[strcspn(line, "\n")] = '\0';

    // Aloque memória para a palavra escolhida aleatoriamente
    char* randomWord = malloc(strlen(line) + 1);
    if (randomWord == NULL) {
        printf("Erro ao alocar memória para a palavra.\n");
        fclose(file);
        exit(1);
    }

    // Copie a palavra escolhida aleatoriamente para a memória alocada
    strcpy(randomWord, line);

    fclose(file);

    return randomWord;
}
