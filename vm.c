#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


#define BUFFER_SIZE 1024
#define STR_EQ(a, b) (strcmp(a, b) == 0)


struct Instruction {
    char* op;
    char* left;
    char* right;
    struct Instruction *next;
};


int r1, r2, r3, r4, r5;
int zf, cf;

struct Instruction* head = NULL;
struct Instruction* tail = NULL;


void error_exit(char* message) {
    puts(message);
    exit(1);
}


struct Instruction*
Instruction_Create(char* op, char* left, char* right) {
    struct Instruction* node = malloc(sizeof *node);

    if (node == NULL)
        error_exit("malloc failed");

    node->op = op;
    node->left = left;
    node->right = right;

    return node;
}


void Instruction_Append(char* op, char* left, char* right) {
    struct Instruction* node = Instruction_Create(op, left, right);

    if (head == NULL)
        head = node;
    else
        tail->next = node;

    tail = node;
}


struct Instruction*
Instruction_FindLabel(char* label) {
    struct Instruction* curr = head;
    char buf[BUFFER_SIZE] = {0};

    strcpy(buf, label);
    strcat(buf, ":");

    while (curr) {
        if (STR_EQ(curr->op, buf))
            return curr;

        curr = curr->next;
    }

    error_exit("label not found");
}


/* not particularly pretty */
char* next(char** code) {
    char  buf[BUFFER_SIZE] = {0};
    char* ptr = buf;
    char* token = NULL;

    if (**code == '\0')
        return NULL;

    /* skip whitespace */
    while (isspace(**code))
        ++(*code);

    while (!isspace(**code) && **code != '\0') {
        *ptr++ = **code;
        ++(*code);
    }

    /* this only happens if \n is immediately followed by \0 */
    if (ptr == buf)
        return NULL;

    if ((token = strdup(buf)) == NULL)
        error_exit("strdup failed");

    return token;
}


int operands_count(char* op) {
    if (STR_EQ(op, "printint"))   return 1;
    if (STR_EQ(op, "printchar"))  return 1;

    if (STR_EQ(op, "je"))  return 1;
    if (STR_EQ(op, "jg"))  return 1;
    if (STR_EQ(op, "jl"))  return 1;
    if (STR_EQ(op, "jne")) return 1;
    if (STR_EQ(op, "jge")) return 1;
    if (STR_EQ(op, "jle")) return 1;
    if (STR_EQ(op, "jmp")) return 1;

    if (STR_EQ(op, "mov")) return 2;
    if (STR_EQ(op, "add")) return 2;
    if (STR_EQ(op, "sub")) return 2;
    if (STR_EQ(op, "mul")) return 2;
    if (STR_EQ(op, "div")) return 2;
    if (STR_EQ(op, "cmp")) return 2;

    return 0;
}


void create_instruction_list(char** code) {
    char* token;
    while (token = next(code)) {
        char* left = NULL;
        char* right = NULL;

        if (operands_count(token) >= 1) left  = next(code);
        if (operands_count(token) == 2) right = next(code);

        Instruction_Append(token, left, right);
    };
}


bool is_number(char* str) {
    while (*str)
        if (!isdigit(*str++))
            return false;

    return true;
}


int* get_reg(char* str) {
    if (STR_EQ(str, "r1")) return &r1;
    if (STR_EQ(str, "r2")) return &r2;
    if (STR_EQ(str, "r3")) return &r3;
    if (STR_EQ(str, "r4")) return &r4;
    if (STR_EQ(str, "r5")) return &r5;

    return NULL;
}


void execute(void) {
    struct Instruction* curr = head;
    int *a, *b;
    int lit_a, lit_b;

    while (curr) {
        if (curr->left) {
            if (is_number(curr->left)) {
                lit_a = atoi(curr->left);
                a = &lit_a;
            } else {
                a = get_reg(curr->left);
            }
        }

        if (curr->right) {
            if (is_number(curr->right)) {
                lit_b = atoi(curr->right);
                b = &lit_b;
            } else {
                b = get_reg(curr->right);
            }
        }

        if (STR_EQ(curr->op, "mov")) *a  = *b;
        if (STR_EQ(curr->op, "add")) *a += *b;
        if (STR_EQ(curr->op, "sub")) *a -= *b;

        if (STR_EQ(curr->op, "mul")) r5 = (*a * *b);
        if (STR_EQ(curr->op, "div")) r5 = (*a / *b);

        if (STR_EQ(curr->op, "cmp")) {
            int result = *a - *b;
            zf = (result == 0);
            cf = (result < 0);
        }

        if (STR_EQ(curr->op, "jmp")) {
            curr = Instruction_FindLabel(curr->left);
            continue;
        }

        if (STR_EQ(curr->op, "jl")) {
            if (cf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "jg")) {
            if (!cf && !zf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "je")) {
            if (zf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "jne")) {
            if (!zf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "jge")) {
            if (zf || !cf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "jle")) {
            if (zf || cf) {
                curr = Instruction_FindLabel(curr->left);
                continue;
            }
        }

        if (STR_EQ(curr->op, "jmp")) {
            curr = Instruction_FindLabel(curr->left);
            continue;
        }

        if (STR_EQ(curr->op, "printint"))  printf("%d", *a);
        if (STR_EQ(curr->op, "printchar")) putchar(*a);
        curr = curr->next;
    }
}


size_t get_filesize(FILE* file_ptr) {
    size_t size;

    fseek(file_ptr, 0, SEEK_END);
    size = ftell(file_ptr);
    rewind(file_ptr);

    return size;
}


char* read_file(char* path) {
    FILE*  file_ptr;
    size_t file_size;
    char*  buf;

    file_ptr = fopen(path, "r");

    if (file_ptr == NULL)
        error_exit("failed to open file");

    file_size = get_filesize(file_ptr);
    buf = calloc(1, file_size + 1);

    if (buf == NULL)
        error_exit("calloc failed");

    fread(buf, 1, file_size, file_ptr);
    fclose(file_ptr);
    return buf;
}


int main(int argc, char **argv) {
    if (argc == 1)
        error_exit("no filename provided");

    char* content = read_file(argv[1]);
    create_instruction_list(&content);
    execute();

    return 0;
}
