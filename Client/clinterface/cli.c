#include "../../Utilities/utility.h"
#include "cli.h"

menu_voice MAIN_MENU[] = {
    {1,"!help"," --> Shows this help."},
    {2,"!who"," --> Shows online clients."},
    {3,"!quit"," --> Leaves the server."},
    {4,"!connect"," username --> Start a match with client \'username\'"}
};
const int MAIN_COUNT = sizeof(MAIN_MENU)/sizeof(menu_voice);

menu_voice GAME_MENU[] = {
    {1,"!help"," --> Shows this help."},
    {2,"!disconnect"," --> Shows online clients."},
    {3,"!shot"," RowCol --> Leaves the server."},
    {4,"!show"," --> Show your and your enemy table"}
};
const int GAME_COUNT = sizeof(GAME_MENU)/sizeof(menu_voice);

int get_menu_key(char* value,menu_voice menu[],int length) {
    int i;
    for(i = 0; i < length; ++i) {
        if(strcmp(menu[i].value,value) == 0)
            return menu[i].key;
    }
    return -1;
}

void discard() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { };
}

char* get_status(user_status u) {
    switch(u) {
        case FREE: return "Free"; break;
        default: return "Busy"; break;
    }
}

void printff(char* str) {
    printf("%s",str);
    fflush(stdout);
}

void print_help(menu_voice menu[],int length) {
    int i = 0;
    printf("\n");
    for(;i < length; ++i) {
        printf("%s%s\n",menu[i].value,menu[i].man);
    }
    printf("\n");
}
