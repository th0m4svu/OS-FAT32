/*
Name: Doungpakanh Paige Keomaxay-Hampf
ID: 1001493622
Name: Thomas Vu
ID: 1001661639
*/

//open command is at line
//CLOSE
//INFO
//STAT
//CD 
//LS
//GET
//read

#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <unistd.h>


#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports five arguments


char *commands[15]; // character pointer array initialized to hold tokenized command entered
int counter = 0;    //this keeps count of commands entered
FILE *fp;
char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;
int32_t RootDirSectors=0;
int32_t FirstDataSector=0;
int32_t FirstSectorofCluster=0;
int ROOTAddress;
bool root_pwd=false;
bool file_is_open=false;
bool match=false;
typedef struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
}DirectoryEntry;

DirectoryEntry dir[16];

int LBAToOffset(int32_t sector)
{
    return ((sector-1)*BPB_BytsPerSec+ROOTAddress);
}

bool compare(char* input,char* file_string)
{
  char *IMG_Name = file_string;

  char expanded_name[12];
  memset(expanded_name, ' ', 12);

  char *token = strtok(input, ".");

  strncpy(expanded_name, token, strlen(token));

  token = strtok(NULL, ".");

  if (token)
  {
    strncpy((char *)(expanded_name + 8), token, strlen(token));
  }

  expanded_name[11] = '\0';

  int i;
  for (i = 0; i < 11; i++)
  {
    expanded_name[i] = toupper(expanded_name[i]);
  }

  if (strncmp(expanded_name, IMG_Name, 11) == 0)
  {
    //printf("%s\t%s\n",expanded_name,IMG_Name);
    return true;
  }

  return false;
}


void print_all_dir()
{
  int i=0;
  for(i=0;i<16;i++)
  {
    printf("\n        Dir_Attr:");
    for (i = 0; i < 16; i++)
        printf("%5x",dir[i].DIR_Attr);
    printf("\nFirstClusterHigh:");
    for (i = 0; i < 16; i++)
      printf("%5x", dir[i].DIR_FirstClusterHigh);
    printf("\n FirstClusterLow:");
    for (i = 0; i < 16; i++)
        printf("%5x", dir[i].DIR_FirstClusterLow);
    printf("\n        FileSize:");
    for (i = 0; i < 16; i++)
      printf("%5x", dir[i].DIR_FileSize);
    }
    printf("\n");
}

void print_closed()
{
  printf("Error: File system image must be opened first.\n");
}

int main()
{
  /*
  int i=0,j=0;
  for(i=0;i<16;i++)
      memset(dir[i].DIR_Name,' ',11);
*/
  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
  
  while (1)
  {
    // Print out the mfs prompt
    printf("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality
    //***********************START SHELL F(X)************************//



    //---------------Continuous Entry-----------------------------//
    if(token[0]==NULL)
      continue;







    //--------------------------EXIT or QUIT--------------------------//
    if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
    {
      printf("Exiting...\n");
      exit(0);
    }


    //-----------------------------OPEN------------------------------//
    if (strcmp(token[0], "open") == 0)
    {
      if(fp!=NULL)
      {
        printf("Error: File system image already open.\n");
        continue;
      }
      else
      {
        fp= fopen(token[1],"r");
        if(fp==NULL)
        {
          printf("Error: File system image not found.\n");
        }
        else if(fp!=NULL)
        {
          fseek(fp, 11, SEEK_SET);
          fread(&BPB_BytsPerSec, 2, 1, fp);

          fseek(fp, 13, SEEK_SET);
          fread(&BPB_SecPerClus, 2, 1, fp);

          fseek(fp, 14, SEEK_SET);
          fread(&BPB_RsvdSecCnt, 2, 1, fp);

          fseek(fp, 16, SEEK_SET);
          fread(&BPB_NumFATs, 2, 1, fp);

          fseek(fp, 36, SEEK_SET);
          fread(&BPB_FATSz32, 4, 1, fp);

          ROOTAddress=(BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);

          fseek(fp, ROOTAddress, SEEK_SET);
          fread(&dir[0], 16, sizeof(DirectoryEntry), fp);
          root_pwd=true;
          file_is_open = true;
        }
      }
    }



    //--printalldir
    else if (strcmp(token[0], "print") == 0)
    {
      print_all_dir();
    }





      //----------------------------CLOSE---------------------------//
      else if ((strcmp(token[0], "close") == 0) && (file_is_open == true))
      {
        if (fp != NULL)
        {
          fclose(fp);
          fp = NULL;
          file_is_open = false;
        }
      }
    
    
    //---------------------------INFO-----------------------------//
    else if ((strcmp(token[0], "info") == 0) && (file_is_open == true))
    {
      printf("                   HEX  DEC\n");
      printf("BPB_BytesPerSec: %5x%5d\n", BPB_BytsPerSec,BPB_BytsPerSec);
      printf(" BPB_SecPerClus: %5x%5d\n", BPB_SecPerClus,BPB_SecPerClus);
      printf(" BPB_RsvdSecCnt: %5x%5d\n", BPB_RsvdSecCnt,BPB_RsvdSecCnt);
      printf("    BPB_NumFATS: %5x%5d\n", BPB_NumFATs, BPB_NumFATs);
      printf("    BPB_FATSz32: %5x%5d\n", BPB_FATSz32, BPB_FATSz32);
    }

    //---------------------------STAT------------------------------//
    else if ((strcmp(token[0], "stat") == 0) && (file_is_open == true))
    {
      if (token[1]==NULL)
      {
        printf("Error: File not found.\n");
      }
      else
      { match=false;
        int i=0;
        for(i=0;i<16;i++)
        {
          char new_name1[101];
          memset(new_name1, 0, 101);
          strncpy(new_name1, token[1], 100);
          char new_name2[101];
          memset(new_name2, 0, 101);
          strncpy(new_name2, dir[i].DIR_Name, 100);
          //printf("%s\t%s\n",token[1],new_name);
          match=compare(new_name1,new_name2);
           
          if (match==true)
          {
            if ((dir[i].DIR_Attr == 0x01) || (dir[i].DIR_Attr == 0x20) || (dir[i].DIR_Attr == 0x10))
            {
              printf("Attribute, Size, Starting Cluster Number\n");
              printf("%d, %d, %d\n\n\n", dir[i].DIR_Attr, dir[i].DIR_FileSize,
                     dir[i].DIR_FirstClusterLow);
              match = false;
            }
          }
        }
      }
    }


    //--------------------------LS---------------------------------//
    else if ((strcmp(token[0], "ls") == 0) && (file_is_open == true))
    {
      int i;
      for (i = 0; i < 16; i++)
      { 
        if ((dir[i].DIR_Attr == 0x01) || (dir[i].DIR_Attr == 0x20) || (dir[i].DIR_Attr == 0x10))
        {
          char new_name[12];
          memset(new_name,0,12);
          strncpy(new_name,dir[i].DIR_Name,11);
          printf("%s\n", new_name);
        }
      }
    }




      //-------------------------CHANGE DIRECTORY-----------------------//
      else if ((strcmp(token[0], "cd") == 0) & (file_is_open == true))
      {
       if(token[1]==NULL) 
       {
         fseek(fp, ROOTAddress, SEEK_SET);
         fread(&dir[0], 16, sizeof(DirectoryEntry), fp);
         root_pwd=true;
         printf("ROOT.\n");
       }
       else
       {
          match = false;
          int i = 0;
          for (i = 0; i < 16; i++)
          {
          char new_name1[101];
          memset(new_name1, 0, 101);
          strncpy(new_name1, token[1], 100);
          char new_name2[101];
          memset(new_name2, 0, 101);
          strncpy(new_name2, dir[i].DIR_Name, 100);
          //printf("%s\t%s\n",token[1],new_name);
          match = compare(new_name1, new_name2);

          if (match == true)
          {
            if (dir[i].DIR_Attr == 0x10)
            {
              fseek(fp, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
              fread(&dir[0],16,sizeof(DirectoryEntry),fp);
              match = false;
            }
          }

        }
       }
      }
      /*else
      {
        print_closed();
      }
      */
    //------------------------------GET--------------------------------//
      //else if((strcmp(token[0], "get") == 0) & (file_is_open == true))
      //{
        
      //}














    //*************************END SHELL F(X)************************//

    int token_index = 0;
    for (token_index = 0; token_index < token_count; token_index++)
    {
      printf("token[%d] = %s\n", token_index, token[token_index]);
    }

    free(working_root);
  }
  return 0;
}
