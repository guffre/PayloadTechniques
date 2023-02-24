#include "modules.h"

unsigned char MODULE_LOADED = 0;

int add(int argc, module_data* argv) {
    if (argc != 2) {
        return -1;
    }

    int x = atoi(argv[0].data);
    int y = atoi(argv[1].data);
    int z = x + y;    
    
    return z;
}

int multiply(int argc, module_data* argv) {
    // code goes here
    return 0;    
}

CommandNode func_add = {NULL, NULL, add, "add", "This function adds two numbers together"};
CommandNode func_multiply = {NULL, NULL, multiply, "multiply", "TODO: This function will multiply two numbers together"};
CommandNode* module_funcs[] = {&func_add, &func_multiply};

__declspec(dllexport) void module_init(CommandNode* command_list) {
    if (!MODULE_LOADED) {
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
                printf("[+] Command Loaded: [%s]\n", module_funcs[i]->func_name);
            }
        }
        MODULE_LOADED = 1;
    }
    else {
        printf("[*] Module already loaded.\n");
    }
}

__declspec(dllexport) void module_unload(CommandNode* command_list) {
    if (!MODULE_LOADED) {
        printf("[!] Error: module not loaded.\n");
    }
    else {
        while(command_list) {
            func_add.prev  = command_list;
            command_list = command_list->next;
        }
        command_list->next = &func_add;
        MODULE_LOADED = 0;
    }
}
