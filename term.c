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

Node *createNode(char *attempt);
void appendNode(Node **head, char *attempt);
void printAttempts(Node *head);
void toUpperCase(char *str);
bool isValidAttempt(char *attempt);
void generateFeedback(char *attempt, char *secretWord, Node *head);
void printColoredLetter(char letter, char colorCode);
void printAttempts(Node *head);
char* chooseRandomWordFromFile();


int main() {
    char* secretWord = chooseRandomWordFromFile();
    Node *head = NULL;
    int attempts = 0;

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
    while (head != NULL) {
        printf("Tentativa: %s - Feedback: ", head->attempt);
        for (int i = 0; i < 5; i++) {
            printColoredLetter(head->attempt[i], head->feedback[i]);
        }
        printf("\n");
        head = head->next;
    }
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
