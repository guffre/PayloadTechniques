#include <stdio.h>
#include <Windows.h>

typedef struct CommandNode CommandNode;
typedef struct module_data module_data;

// Doubly linked list for commands to be listed in
struct CommandNode {
    CommandNode* prev;
    CommandNode* next;
    void*        func;
    char*        func_name;
    char*        description;
};

// All modules will take their arguments as this type
struct module_data{
    unsigned int length;
    unsigned char* data;
};

typedef void(*_module_init) (CommandNode*, HMODULE);
typedef  int(*_module_cmd)  (int, module_data*);

void print_command_list(CommandNode*);
void load_module(CommandNode*, char*);

// Prints out the list of commands
// `command_list` can be any node in the list of commands
void print_command_list(CommandNode* command_list) {
    printf("\n=== Available Commands ===\n");
     // Rewinds the list to reach the head node
    while (command_list && command_list->prev) {
        command_list = command_list->prev;
    }
    for (; command_list; command_list = command_list->next) {
        printf("Command [%p]: %s\n", command_list, command_list->func_name);
    }
    printf("=== End of list ===\n\n");
}

// Default function to load a module.
void load_module(CommandNode* command_list_head, char* module_name) {
    printf("[+] Loading module: %s\n", module_name);
    HMODULE module = LoadLibraryA(module_name);
    _module_init module_init = (_module_init)GetProcAddress(module, "module_init");
    module_init(command_list_head, module);
}

// Default function to unload a module.
void unload_module(CommandNode* command_list_head, char* module_name) {
    printf("[+] Unloading module: %s\n", module_name);
    _module_init module_unload = (_module_init)GetProcAddress(LoadLibraryA(module_name), "module_unload");
    module_unload(command_list_head, NULL);
}
