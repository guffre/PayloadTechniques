#include "modules.h"

// Global module handle, also indicates if module is loaded
HMODULE MODULE = NULL;

// Sample command function
int add(int argc, module_data* argv) {
    if (argc != 2) {
        return -1;
    }

    int x = atoi(argv[0].data);
    int y = atoi(argv[1].data);
    int z = x + y;    
    
    return z;
}

// Another sample command
int multiply(int argc, module_data* argv) {
    // code goes here
    return 0;    
}

CommandNode func_add = {NULL, NULL, add, "add", "This function adds two numbers together"};
CommandNode func_multiply = {NULL, NULL, multiply, "multiply", "TODO: This function will multiply two numbers together"};

// This is an array of all functions that the module will provide to the client
CommandNode* module_funcs[] = {&func_add, &func_multiply};


/* 
 * All code below this line is boiler-plate 
 * These functions provides load/unload functionality
 */

// Default functionality to load a modules functions into the command_list
__declspec(dllexport) void module_init(CommandNode* command_list, HMODULE module) {
    if (!MODULE) {
        CommandNode* current = command_list;
        while (command_list) {
            current = command_list;
            command_list = command_list->next;
        }

        for (int i = 0; i < sizeof(module_funcs)/sizeof(module_funcs[0]); i++) {
            if (current == module_funcs[i]) {
                printf("[!] ERROR: Command [%s] has already been loaded.\n", current->func_name);
            }
            else {
                module_funcs[i]->prev = current;
                module_funcs[i]->next = NULL;
                current->next = module_funcs[i];
                current = module_funcs[i];
                printf("[+] Command Loaded: [%s][%p]\n", module_funcs[i]->func_name, module_funcs[i]->func);
            }
        }
        MODULE = module;
    }
    else {
        printf("[*] Module already loaded.\n");
    }
}

// Default functionality to unload a modules functions from the command list
// Also frees the module from memory
__declspec(dllexport) void module_unload(CommandNode* command_list) {
    if (!MODULE) {
        printf("[!] Error: module not loaded.\n");
    }
    else {
        CommandNode* current = command_list;
        while (command_list) {
            current = command_list;
            command_list = command_list->next;
            for (int i = 0; i < sizeof(module_funcs)/sizeof(module_funcs[0]); i++) {
                if (current == module_funcs[i]) {
                    // Sets the previous nodes "next" forward one node, or NULL
                    current->prev->next = current->next;
                    if (current->next)
                        current->next->prev = current->prev;
                    // No need to keep looping once theres a match
                    break;
                }
            }
        }
        FreeLibrary(MODULE);
        MODULE = NULL;
    }
}
