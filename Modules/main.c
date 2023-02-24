#include "modules.h"

int sub(int argc, module_data* argv) {
    if (argc != 2) {
        return -1;
    }

    int x = atoi(argv[0].data);
    int y = atoi(argv[1].data);
    int z = x - y;    
    
    return z;
}
CommandNode func_sub = {NULL, NULL, sub, "sub", "This function subtracts two numbers"};

int main(int argc, char** argv) {
    CommandNode* command_list;
    command_list = &func_sub;

    print_command_list(command_list);
    load_module(command_list, "module_example.dll");
    load_module(command_list, "module_example.dll");
    print_command_list(command_list);

    module_data tmp[2];
    tmp[0] = (module_data){1, "15"};
    tmp[1] = (module_data){1, "5"};

    printf("Calling each function in command list, with arguments %s and %s\n", tmp[0].data, tmp[1].data);
    for (; command_list; command_list = command_list->next) {
        printf("Command: %s\n",       command_list->func_name);
        printf("\tDescription: %s\n", command_list->description);
        int result = ((_module_cmd)command_list->func)(2, tmp) ;
        printf("\t %s(%s, %s) -> %d\n", command_list->func_name, tmp[0].data, tmp[1].data, result);
    }
}
