#include <stdio.h>
#include <Windows.h>

typedef struct CommandNode CommandNode;
typedef struct module_data module_data;

struct CommandNode {
    CommandNode* prev;
    CommandNode* next;
    void*        func;
    char*        func_name;
    char*        description;
};

struct module_data{
    unsigned int length;
    unsigned char* data;
};

typedef void(*_module_init) (CommandNode*);
typedef  int(*_module_cmd)  (int, module_data*);

void print_command_list(CommandNode*);
void load_module(CommandNode*, char*);


void print_command_list(CommandNode* command_list) {
    printf("=== Available Commands ===\n");
    for (; command_list; command_list = command_list->next) {
        printf("Command [%p]: %s\n", command_list, command_list->func_name);
    }
    printf("=== End of list ===\n");
}

void load_module(CommandNode* command_list_head, char* module_name) {
    printf("[+] Loading module: %s\n", module_name);
    _module_init module_init = (_module_init)GetProcAddress(LoadLibraryA(module_name), "module_init");
    module_init(command_list_head);
}