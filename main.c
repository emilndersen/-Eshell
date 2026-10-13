#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITERS " \t\r\n\a"

// Function to print banner
void printWelcome() {
    printf("\n************************************************\n\n");
    printf("  **************** EShell ****************\n\n");
    printf("  \"Meow Meow Meow Meow Meow Meow Meow Meow\"  \n\n");
    printf("**************************************************\n\n");
}

// Парсинг строки на аргументы
void parse_input(char *input, char **args){
    // Разбиваем слова на токены
    char *token = strtok(input, DELIMITERS);
    // индекс текущего аргумента
    int i = 0;
    // Пока есть токен и место в массиве
    while(token != NULL && i < MAX_ARGS - 1){
        args[i++] = token; // сохраняем токен и сдвигаем индекс
        token = strtok(NULL, DELIMITERS); // Берём след. токен
    } 
    args[i] = NULL; // завершаем массив аргументов NULL
}

void execute_pipe(char  *left_str, char *right_str){
    
    // Массивы для каждой половины пайпа
    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];

    // Вызов функции parse_input
    parse_input(left_str, args1);
    parse_input(right_str, args2);

    // Если хотя бы с одной стороны пусто, завершаем функцию
    if(args1[0] == NULL || args2[0] == NULL){
        return;
    }


    int pipefd[2]; // файловый дескриптор
    // Создаем канал связи
    if(pipe(pipefd) == -1){
        // Ошибка, если -1
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if(pid1 == 0){
        // Первый дочерний процесс (Левая команда): пишет в пайп
        dup2(pipefd[1], STDOUT_FILENO); // Перенаправляем stdout в пайп
        close(pipefd[0]);   // Закрываем неиспользуемый конец на чтении
        close(pipefd[1]);   // Закрываем оригинальный дескриптор

        execvp(args1[0], args1);
        perror(args1[0]); // Если execvp упадет 
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if(pid2 == 0){
        // Второй дочерний процесс (Правая команда): читает из пайпа
        dup2(pipefd[0], STDIN_FILENO); 
        close(pipefd[0]);   
        close(pipefd[1]);   

        execvp(args2[0], args2);
        perror(args2[0]);  
        exit(EXIT_FAILURE);
    }

    // Родительский процесс закрывает оба конца пайпа, чтобы процессы не зависли
    close(pipefd[0]);
    close(pipefd[1]);

    // Ждем завершения обоих процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main(void) {

    printWelcome();
    char *input;
    char *args[MAX_ARGS];
    char cwd[1024];

    char prompt[1040];

    while (1) {

        // Получаем текущую директорию
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            // Безопасно собираем строку promt
            snprintf(prompt, sizeof(prompt), "eshell:%s $ ", cwd);
        }else{
             // На случай ошибки getcwd используем фолбек
             snprintf(prompt, sizeof(prompt), "eshell:unknown$ ");
        }

        // Использование readline для вывода prompt и чтения строки
        input = readline(prompt);

        // Проверка на Ctrl+D (EOF)
        if (input == NULL) {
            printf("\nПОка епта\n");
            break;
        }

        // Пропускаем пустые строки
        if (input[0] == '\0') {
            free(input);
            continue;
        }

        // Добавляем команду в историю стрелочек
        add_history(input);
        
        // Обнаружение пайпа
        char *pipe_pos = strchr(input, '|');
        if(pipe_pos != NULL){
            *pipe_pos = '\0';
            char *left = input;
            char *right = pipe_pos + 1;

            // Вызываем нашу функцию для пайпов
            execute_pipe(left, right);

            free(input);
            continue;   // Возвращаемся в начало цикла shell
        }

        // --- ЛОГИКА ДЛЯ ОДИНОЧНЫХ КОМАНД ---
        // Парсинг строки на аргументы (если пайпа нет)
        parse_input(input, args);

        // Если пользователь ввёл только пробелы
        if(args[0] == NULL){
            free(input);
            continue;
        }

        // EOF (не только Ctrl + D)
        if(strcmp(args[0], "exit") == 0){
            free(input);
            break;
        }

        if(strcmp(args[0], "cd") == 0){
            if(args[1] == NULL){
                fprintf(stderr, "cd: ожидается аргумент\n");
            }else{
                if(chdir(args[1]) != 0){
                    perror("cd");
                }
            }
            free(input);
            continue;
        }
        
        pid_t pid = fork();
        if(pid == 0){
            execvp(args[0], args);
            perror(args[0]);

            exit(EXIT_FAILURE);            
        }else{
            wait(NULL);
        }

        // Освобождаем память
        free(input);
    }
}