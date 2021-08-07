
#include <stdint.h> // uint8_t, uintptr_t
#include <stdio.h> // printf, putc, getc, stdin, stdout, FILE, fopen, fseek
#include <stdlib.h> // free, malloc, realloc, exit


#define RAM_SIZE            0x1000 // 4KB of memory
#define STACK_BLOCK_SIZE    10 // how much to increment stack size on realloc


enum {
    OP_PLUS    = '+',
    OP_COMMA, // ','
    OP_MINUS, // '-'
    OP_DOT,   // '.'
    OP_LT      = '<',
    OP_GT      = '>',
    OP_LBR     = '[',
    OP_RBR     = ']'
};

typedef uint8_t byte;

byte*       code; // place to store code from file
byte        memory[RAM_SIZE]; // memory we can manipulate
uintptr_t*  stack; // place to store PC address for loop logic
int         stack_blocks; // how many addresses will fit on the stack
int         stack_size; // how many addresses currently in stack


int         read_file(const char*);
void*       xmalloc(size_t);
void        init_stack();
void*       xrealloc(void*, size_t);
void        stack_push(uintptr_t);
uintptr_t   stack_peek();
uintptr_t   stack_pop();


int main(int argc, const char** argv) {
    if(argc < 2) {
        printf("usage: bf file\n");
        return 0;
    }
    
    if(!read_file(argv[1])) {
        printf("failed to read file %s\n", argv[1]);
        return -1;
    }
    
    init_stack();
    byte* pc = code; // our place in code
    byte* ptr = memory; // our place in memory
    while(*pc) {
        switch(*pc) {
            case OP_PLUS: // increment value of a cell in memory
                (*ptr)++;
                break;
            case OP_MINUS: // decrement value of a cell in memory
                (*ptr)--;
                break;
            case OP_GT: // move to next cell in memory
                ptr++; // avoid going out of bounds
                if(ptr - memory >= RAM_SIZE) ptr = memory + RAM_SIZE - 1;
                break;
            case OP_LT: // move to previous cell in memory
                ptr--; // avoid going out of bounds
                if(ptr < memory) ptr = memory;
                break;
            case OP_DOT: // output value of current cell to stdout as char
                putc(*ptr, stdout);
                break;
            case OP_COMMA: // set value of current cell to value read from stdin
                *ptr = getc(stdin);
                break;
            case OP_LBR: // if val of current cell != 0 add pc to stack
                if(*ptr) stack_push((uintptr_t) pc);
                else { // otherwise we skip to the end of current loop
                    int closing = 1; // how many nested braces we're in
                    while(closing) {
                        pc++;
                        if(*pc == OP_LBR) closing++;
                        if(*pc == OP_RBR) closing--;
                        if(!*pc) { // exit if we reach the end of code
                            printf("\nreached eof while searching for ]\n");
                            exit(-1);
                        }
                    }
                }
                break;
            case OP_RBR: // if val of current cell != 0 return to matching brace
                // if we reach this op with nothing on stack, skip it
                if(*ptr && stack_size) pc = (byte*) stack_peek();
                else stack_pop(); // otherwise remove address from stack
                break;
        }
        pc++; // go to next instruction
    }
    free(code); // free mem we allocated for code and stack
    free(stack);
    
    printf("\n\ndone\n\n"); // we're done
    return 0;
}


int read_file(const char* path) {
    FILE* file = fopen(path, "r");
    if(!file) return 0; // failed to read file
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file); // seek to end to get file size
    fseek(file, 0L, SEEK_SET);
    code = (byte*) xmalloc(size + 1); // allocate size bytes for code +1 for \0
    size_t read = fread(code, 1, size, file); // copy file into code buffer
    //printf("read %u bytes from %s\n", read, path);
    code[read] = '\0'; // add null terminator to code buffer
    fclose(file);
    return 1; // success
}

void* xmalloc(size_t size) { // safe malloc exits program if fails
    void* p = malloc(size);
    if(!p) {
        printf("failed to allocate %lu bytes of memory\n", size);
        exit(-1);
    }
    return p;
}

void init_stack() { // allocate memory for address stack
    stack_blocks = STACK_BLOCK_SIZE;
    stack_size = 0;
    stack = (uintptr_t*) xmalloc(stack_blocks * sizeof(uintptr_t));
}

void stack_push(uintptr_t addr) {
    int index = stack_size + 1;
    if(index > stack_blocks) { // if index is out of bounds allocate more mem
        stack_blocks += STACK_BLOCK_SIZE;
        stack = (uintptr_t*) xrealloc(stack, stack_blocks * sizeof(uintptr_t));
    }
    stack[++stack_size] = addr; // add address to stack
}

void* xrealloc(void* p, size_t size) { // safe realloc
    p = realloc(p, size);
    if(!p) {
        printf("failed to reallocate %lu bytes of memory\n", size);
        exit(-1);
    }
    return p;
}

uintptr_t stack_peek() { // view top address in stack without removing
    if(stack_size == 0) return 0;
    return stack[stack_size];
}

uintptr_t stack_pop() { // remove top address in stack
    if(stack_size == 0) return 0;
    return stack[stack_size--];
}
