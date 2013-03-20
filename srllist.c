#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include "srllist.h"

/*
 * To build release:
 *      #make
 * To build debug:
 *      #make debug
 * To clean build 
 *      #make clean
 *
 * usage:
 *      srllist [filename]
 *
 * Existing file will be loaded into the program. Otherwise, new file will be created.
 *
 *
 * in program usage:
 *      addlist                  -- create new list with automatic assigned ID. old IDs are recycled.
 *      showlist   [ID]          -- display list with given ID
 *      addnode    [ID] [Value]  -- add node with given value to list with given ID
 *      removenode [ID] [Value]  -- remove node with given value from list with given ID
 *      removelist [ID]          -- remove list with given ID
 *      exit                     -- leave the program
 */

int main(int argc, char* argv[]){


    File cur_file = NULL;
    int tar_ID;
    int tar_data;
    char* filename = NULL;

    if (argc!=2){
        usage();
        exit(0);
    }else{
        filename = argv[1];
        FILE* fp = fopen(filename, "r");
        if (fp!=NULL) {
            cur_file = deserialize(filename);
            fclose(fp);
        }else{
            cur_file = calloc(1, sizeof(struct FileStruct));
            cur_file->node_count=0;
            cur_file->list_count=0;
        }
    }
    
    printf("srllist> ");
    char* cmd=getLine();
    while (strncmp(cmd, "exit", 4)!=0){
        char arg1[20], arg2[20], arg3[20];
        memset(arg1,'\0',20);
        memset(arg2,'\0',20);
        memset(arg3,'\0',20);
        /* add list */
        if (strncmp(cmd, "addlist", 7)==0){
            addList(cur_file);
        }else /* show list */
        if (strncmp(cmd, "showlist",8)==0){
            sscanf(cmd, "%s %s", arg1, arg2);
            sscanf(arg2, "%d", &tar_ID);
            displayList(cur_file, tar_ID);
        }else /* add node */
        if (strncmp(cmd, "addnode",7)==0){
            sscanf(cmd, "%s %s %s", arg1, arg2, arg3);
            sscanf(arg2, "%d",&tar_ID);
            sscanf(arg3, "%d",&tar_data);
            addNode(cur_file, tar_ID, tar_data);
        }else /* remove node */
        if (strncmp(cmd, "removenode",10)==0){
            sscanf(cmd, "%s %s %s", arg1, arg2, arg3);
            sscanf(arg2, "%d",&tar_ID);
            sscanf(arg3, "%d",&tar_data);
            removeNode(cur_file, tar_ID, tar_data);
        }else /* remove list */
        if (strncmp(cmd, "removelist",10)==0){
            sscanf(cmd, "%s %s", arg1, arg2);
            sscanf(arg2, "%d", &tar_ID);
            removeList(cur_file, tar_ID);
        }else{ /* print tip */
            usage();
        }
        free(cmd);

        char* buffer = serialize(cur_file);
        flush(filename,buffer,cur_file);

        printf("srllist> ");
        cmd=getLine();
    }
    
    free(cmd);
    char* buffer = serialize(cur_file);
    flush(filename,buffer,cur_file);
    cleanup(cur_file);

    return 0;
}

char* serialize(File cur_file){
    /* allocate buffer for file write */
    /* 7 bytes = 1 byte for sign/List marker + 5 bytes to int16 + 1 byte for NULL character */
    char* buffer = calloc((cur_file->node_count+cur_file->list_count)*7, sizeof(char));
    int loc=0;
    /* write lists into file */
    List cur_list = cur_file->first_list;
    while (cur_list != NULL){

        /* write list sentinel */
        buffer[loc]='L'; 
        sprintf(buffer+loc+1, "%u", cur_list->ID);
        loc+=7;

        /* write nodes into file */
        Node cur_node = cur_list->head;
        while (cur_node != NULL){
            sprintf(buffer+loc, "%i", cur_node->data);
            loc+=7;
            cur_node = cur_node->next;
        }
        /* update current list */
        cur_list = cur_list->next;
    }

    return buffer;
}

void flush(char* filename, char* buffer, File cur_file){
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Couldn't open data file.\n");
        free(buffer);
        return;
    }
    
    /* write number of nodes in the file */
    fprintf(fp, "%d\n", cur_file->node_count+cur_file->list_count);

    int i;
    for (i=0;i<(cur_file->node_count + cur_file->list_count)*7; i+=7){
        fprintf(fp,"%s\n",buffer+i);
    }

    fclose(fp);

    /* free buffer */
    free(buffer);
}


File deserialize(char* filename){

    /* init File struct */
    File cur_file = calloc(1,sizeof(struct FileStruct));
    cur_file->list_count=0;
    cur_file->node_count=0;
    cur_file->first_list=NULL;

    /* open file for read */
    FILE *fp;
    int line_count,cur_line;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Couldn't open data file.\n");
        return NULL;
    }
   
    char line[8];
    /* get number of lines */
    fgets(line, sizeof(line), fp);
    sscanf(line, "%d", &line_count);
    
    /* prepare to chain lists and nodes */
    List pre_list=NULL;
    Node pre_node=NULL;
    List cur_list=NULL;
    Node cur_node=NULL;


    /* read each line */
    for (cur_line=0; cur_line < line_count; cur_line++){
        fgets(line, sizeof(line), fp);
        line[strlen(line)-1]=0;

        /* if found a new list */
        if (line[0]=='L'){
            pre_list = cur_list;
            cur_list = calloc(1, sizeof(struct ListStruct));
            sscanf(line+1,"%u",&(cur_list->ID));
            cur_list->node_count=0;
            cur_list->next=NULL;
            cur_list->head=NULL;

            if (pre_list!=NULL) {
                pre_list->next = cur_list;
            }else{
                cur_file->first_list=cur_list;
            }
            cur_file->list_count++;

        }

        /* if found a new node */
        else {
            pre_node = cur_node;
            cur_node = calloc(1, sizeof(struct NodeStruct));
            sscanf(line,"%i",&(cur_node->data));
            /* if it is the first node*/
            if (cur_list->head==NULL) {
                cur_list->head=cur_node;
            }else{
            /* if it is not the first node*/
                pre_node->next = cur_node;
            }

            cur_list->node_count++;
            cur_file->node_count++;

        }
        

    }

    fclose(fp);
    return cur_file;
}

void cleanup(File file){
    List pre_list = NULL;
    List cur_list = NULL;
    Node pre_node = NULL;
    Node cur_node = NULL;
    cur_list = file->first_list;
    while (cur_list!=NULL) {
        cur_node=cur_list->head;
        while (cur_node!=NULL) {
            pre_node=cur_node;
            cur_node=cur_node->next;
            free(pre_node);
        }
        pre_list=cur_list;
        cur_list=cur_list->next;
        free(pre_list);
    }
    free(file);
}

List getList(File cur_file, uint16_t tar_ID){
    List cur_list = cur_file->first_list;
    while (cur_list != NULL) {
       if (cur_list->ID == tar_ID){
           return cur_list;
       }else{
           cur_list=cur_list->next;
       }
    }
    return NULL;
}

Node getNode(File cur_file, uint16_t tar_ID, int16_t tar_data){

    List cur_list = getList(cur_file, tar_ID);
    if (cur_list == NULL) {
        return NULL;
    }else{
        Node cur_node = cur_list->head;
        while (cur_node != NULL){
           if (cur_node->data > tar_data){
                return NULL;
           }else if (cur_node->data == tar_data){
                return cur_node;
           }else{
                cur_node = cur_node -> next;
           }
        }
    }
    return NULL;
} 

void addList(File cur_file){

    List new_list = calloc(1,sizeof(struct ListStruct)); 
    new_list->node_count=0;

    /* recycle possible ID's */
    uint16_t cur_ID=0;
    List pre_list=cur_file->first_list;
    if (pre_list==NULL || pre_list->ID > 0){
        new_list->next = cur_file->first_list;
        cur_file->first_list = new_list;
        new_list->ID=0;
    }else{
        while (pre_list->next != NULL && pre_list->next->ID == pre_list->ID + 1){
            pre_list = pre_list->next;
        }
        new_list->ID = pre_list->ID + 1;
        new_list->next = pre_list->next;
        pre_list->next = new_list;
    }

    cur_file->list_count++;
  
    printf("New list added, ID=%u\n", new_list->ID);
}

    
void displayList(File cur_file, uint16_t tar_ID){
    List cur_list = getList(cur_file, tar_ID);
    if (cur_list == NULL) {
        printf("List doesn't exist!\n");
        return;
    }else{
        Node cur_node = cur_list->head;
        while (cur_node != NULL) {
            printf("%d\n", cur_node->data);
            cur_node = cur_node -> next;
        }
    }
}

void addNode(File cur_file, uint16_t tar_ID, int16_t new_data){
    List cur_list = getList(cur_file, tar_ID);
    if (cur_list == NULL) {
        printf("List doesn't exist!\n");
        return;
    }else{
        /* allocate new node */
        Node new_node = calloc(1,sizeof(struct NodeStruct)); 
        new_node->data = new_data;

        /* search for the right position, keep list in order. */
        /* if new node is smallest */
        Node pre_node = cur_list->head;
        if (pre_node == NULL || new_data < pre_node->data) {
            new_node->next = pre_node;
            cur_list->head = new_node;
        }else{
            /* find the biggest node that is smaller than the new node */
            while(pre_node->next != NULL && pre_node->next->data < new_data){
                pre_node = pre_node -> next;
            }
            new_node->next = pre_node->next;
            pre_node->next = new_node;
        }

        /* increase node count*/
        cur_list->node_count++;
        cur_file->node_count++;
    }
}

void removeNode(File cur_file, uint16_t tar_ID, int16_t tar_data){

    List cur_list = getList(cur_file, tar_ID);
    if (cur_list == NULL) {
        printf("List doesn't exist!\n");
        return;
    }else{
        Node tar_node = getNode(cur_file, tar_ID, tar_data);
        if (tar_node == NULL) {
            printf("Target node foesn't exist in the given list!\n");
        }else{
            /*if the node is the smallest node */
            if (cur_list->head->data == tar_data){
                cur_list->head = tar_node->next;
                free(tar_node);
            }else{
                /*find preceding node*/
                Node pre_node = cur_list->head;
                while(pre_node->next != tar_node){
                    pre_node=pre_node->next;
                }
                
                /*fix pointers*/
                pre_node->next = tar_node->next;
                free(tar_node);
            }

            /* decrease node count*/
            cur_list->node_count--;
            cur_file->node_count--;
        }
    }
}


void removeList(File cur_file, uint16_t tar_ID){

    List cur_list = getList(cur_file, tar_ID);
    if (cur_list == NULL) {
        printf("List doesn't exist!\n");
        return;
    }else{

        /* fix pointer from preceding list */
        if (cur_file -> first_list == cur_list){
            cur_file -> first_list = cur_list -> next; 
        }else{
            List pre_list = cur_file-> first_list;
            while (pre_list->next != cur_list) {
                pre_list = pre_list -> next;
            }

            pre_list->next = cur_list->next;
        }

        /* free nodes */
        Node pre_node=NULL;
        Node cur_node=cur_list->head;
        while (cur_node!=NULL) {
            pre_node=cur_node;
            cur_node=cur_node->next;
            free(pre_node);
        }
       
        /* decrease counter */
        cur_file->node_count -= cur_list->node_count;
        cur_file->list_count --;
        
        /* free list */
        free(cur_list);
    }
}

void usage(void){
    printf("To start program:\n      srllist [filename]\nExisting file will be opened, otherwise, new file is created\n");
    printf("In program:\n      addlist -- create a new list\n      showlist [ID] -- print a given list\n      addnode [ID] [Value] -- add node to list with given ID\n      removenode [ID] [Value] -- remove node from given list\n      removelist [ID] -- remove given list\n      exit -- exit the program\n");
}

char* getLine(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}
