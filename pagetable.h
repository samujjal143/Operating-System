typedef struct
{
	int valid;
	int frame;
	int dirty;
	int request;
	int reference;
}page_table;
typedef page_table* pt;
