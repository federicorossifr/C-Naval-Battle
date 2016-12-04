typedef struct menu_voice {
	int key;
	char* value;
	char* man;
} menu_voice;

int get_menu_key(char* value,menu_voice menu[],int length);
void discard();
char* get_status(user_status u);
void printff(char* str);
void print_help(menu_voice menu[],int length);
