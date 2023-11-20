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
#define ANSI_COLOR_RESET    "\x1b[0m"

// Lista encadeada
typedef struct Node {
    char attempt[6];
    char feedback[6];
    struct Node *next;
} Node;


typedef struct {
    char playerName[20];
    int attempts;
} RankingEntry;

Node *createNode(char *attempt);
void appendNode(Node **head, char *attempt);
void printAttempts(Node *head);
void toUpperCase(char *str);
bool isValidAttempt(char *attempt);
void generateFeedback(char *attempt, char *secretWord, Node *head);
void printColoredLetter(char letter, char colorCode);
void printAttempts(Node *head);
char* chooseRandomWordFromFile();
void loadRanking(RankingEntry **ranking, int *numEntries);
void updateRanking(RankingEntry *ranking, int numEntries, char *playerName, int attempts);


int main() {
    char* secretWord = chooseRandomWordFromFile();
    Node *head = NULL;
    int attempts = 0;



        // Crie um array de estruturas RankingEntry para armazenar o ranking
    RankingEntry *ranking = NULL;
    int numRankingEntries = 0;

    // Carregue o ranking existente do arquivo
    loadRanking(&ranking, &numRankingEntries);

    printf("Bem-vindo ao Term!\n");
    printf("Voce tem 6 tentativas para adivinhar uma palavra de 5 letras.\n");

    while (attempts < 6) {
        char userAttempt[10];  // Buffer maior para validar o input 
        printf("Tentativa %d: ", attempts + 1);
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

        appendNode(&head, userAttempt);
        generateFeedback(userAttempt, secretWord, head);

        printAttempts(head);

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
    updateRanking(ranking, numRankingEntries, playerName, attempts);

    // Liberando a memória
    while (head != NULL) {
        Node *temp = head;
        head = head->next;
        free(temp);
    }

    printf("Deseja jogar novamente? (s/n) ");
    char answer;
    scanf("%s", &answer);
    if (answer == 's') {
        main();
    }

    return 0;
}

// Criar novo nó
Node* createNode(char *attempt) {
    Node *newNode = (Node*) malloc(sizeof(Node));
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
void appendNode(Node **head, char *attempt) {
    Node *newNode = createNode(attempt);
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node *temp = *head;
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
void generateFeedback(char *attempt, char *secret, Node *head) {
    Node *current = head;
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
void printAttempts(Node *head) {
    Node *temp = head;
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


void loadRanking(RankingEntry **ranking, int *numEntries) {
    FILE *file = fopen("ranking.txt", "r");
    if (file == NULL) {
        // Se o arquivo não existe, não há entradas no ranking
        return;
    }

    // Inicializa o número de entradas no ranking
    *numEntries = 0;

    // Aloca memória inicial para o ranking
    *ranking = malloc(sizeof(RankingEntry));
    if (*ranking == NULL) {
        printf("Erro ao alocar memória para o ranking.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Loop para ler as entradas do arquivo
    while (fscanf(file, "%s %d", (*ranking + *numEntries)->playerName, &(*ranking + *numEntries)->attempts) == 2) {
        (*numEntries)++;
        // Realoca memória para o ranking
        *ranking = realloc(*ranking, (*numEntries + 1) * sizeof(RankingEntry));
        if (*ranking == NULL) {
            printf("Erro ao alocar memória para o ranking.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
}


// Função para atualizar o ranking
void updateRanking(RankingEntry *ranking, int numEntries, char *playerName, int attempts) {
    ranking = realloc(ranking, (numEntries + 1) * sizeof(RankingEntry));
    if (ranking == NULL) {
        printf("Erro ao alocar memória para o ranking.\n");
        exit(EXIT_FAILURE);
    }

    // Adiciona a nova entrada no final
    strcpy((ranking + numEntries)->playerName, playerName);
    (ranking + numEntries)->attempts = attempts;
    numEntries++;

    // Ordena o ranking usando o algoritmo Bubble Sort
    for (int i = 0; i < numEntries - 1; i++) {
        for (int j = 0; j < numEntries - 1 - i; j++) {
            if ((ranking + j)->attempts > (ranking + j + 1)->attempts) {
                // Troca as posições se o número de tentativas for maior
                RankingEntry temp = *(ranking + j);
                *(ranking + j) = *(ranking + j + 1);
                *(ranking + j + 1) = temp;
            }
        }
    }

    // Salva o ranking atualizado no arquivo
    FILE *file = fopen("ranking.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de ranking.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numEntries; i++) {
        fprintf(file, "%s %d\n", (ranking + i)->playerName, (ranking + i)->attempts);
    }

    fclose(file);
}
