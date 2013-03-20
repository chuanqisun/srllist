#include<stdint.h>
/* ---------------------STRUCT --------------------------*/

typedef struct NodeStruct{
    int16_t data;
    struct NodeStruct *next;
}* Node;

typedef struct ListStruct{
    uint16_t ID;
    uint16_t node_count;
    struct ListStruct *next;
    struct NodeStruct *head;
}* List;

typedef struct FileStruct{
    uint16_t node_count;
    uint16_t list_count;
    struct ListStruct* first_list;
}* File;


/* ---------------------PROTOTYPE -----------------------*/

/* write all existing data structures from cur_file into the buffer */
char* serialize(File cur_file);

/* write the buffer into a file with given name */
void flush(char* filename, char* buffer, File cur_file);

/* load data structure from file */
File deserialize(char* filename);

/* free dynamic memory */
void cleanup(File file);

/* get List from ID */
List getList(File cur_file, uint16_t ID);

/* get Node from ID and data value */
Node getNode(File cur_file, uint16_t tar_ID, int16_t data);

/* add a new list */
void addList(File file);

/* display a given list */
void displayList(File cur_file, uint16_t tar_ID);

/* add node to given list */
void addNode(File cur_file, uint16_t tar_ID, int16_t tar_data);

/* remove node from a given list */
void removeNode(File cur_file, uint16_t tar_ID, int16_t tar_data);

/* remove list */
void removeList(File cur_file, uint16_t tar_ID);

/* print usage tip */
void usage(void);

/* get a new line from console */
char* getLine(void);
