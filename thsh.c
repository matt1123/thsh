/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <fcntl.h>


// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define SHELL "/bin/sh"


bool used = false;
int 
main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "thsh> ";
  char cmd[MAX_INPUT];

  char tkncmd[1024];
  char dbgcmd[1024];
  char* token;


  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;


    char* cwd;
    char buff[PATH_MAX + 1];
    cwd = getcwd (buff, PATH_MAX+1);

    // Print the prompt
    write(1, "[", strlen("["));
    write(1, cwd, strlen(cwd));
    write(1, "]", strlen("]"));
    
    rv = write(1, prompt, strlen(prompt));
    // rv = printf("%s", prompt);
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, 
	  cursor = cmd, last_char = 1;
	rv 
	  && (++count < (MAX_INPUT-1))
	  && (last_char != '\n');
	cursor++) { 

      rv = read(0, cursor, 1);
      last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }


    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    // write(1, cmd, strnlen(cmd, MAX_INPUT));




// SCRIPTING INPUT
    if(argc>1){
      char *inpt = argv[1];
      if(strstr(inpt, ".sh") != NULL){
        char* chmod = NULL;
        strcpy(chmod, "chmod +x ");
        strcat(chmod, inpt);

        system(chmod);

        char* dotSl = NULL;
        strcpy(dotSl, "./");
        strcat(dotSl,inpt);
        
        strcpy(cmd, dotSl);
        used = true;
      }else{
        
        strcpy(cmd, inpt);
       
        if(argc>2){
          strcat(cmd, " ");
          strcat(cmd, argv[2]);
          if(argc>3){
            strcat(cmd, " ");
            strcat(cmd, argv[3]);
            if(argc>3){
              strcat(cmd, " ");
              strcat(cmd, argv[4]);
            }
          }
        }
      }
    }



    

// TOKENIZE INPUT
    char s[1024];
    strcpy(s, cmd);
    token = strtok(s, " ");

// DEBUG BEGIN
  bool debug = false;
  if(strstr(cmd,"-d")!=NULL){
    debug = true;
    
    token = strtok(NULL, "-");
    
    
    printf("%s", "RUNNING:");
    printf("%s\n",token);
    // strcpy(dbgcmd, token);
    // strcpy(token, token);
    strcpy(cmd, token);
    fflush(stdout);
  }

// HANDLE COMMENT
    if(strstr(s,"#")){
      goto END;
    }


// BACKGROUND OPERATIONS
    if(strstr(cmd,"&")!=NULL || strstr(cmd,"bg")!=NULL){
      char* cmdBG = strtok(cmd, "&");
      pid_t pid = fork();
      if (pid < 0){
          // error, failed to fork()
      }else if (pid > 0){
        int status;
        waitpid(pid, &status, WNOHANG);
      }else if (pid==0){
        execl(SHELL, SHELL, "-c", cmdBG, NULL);
        exit(EXIT_SUCCESS);
      } 


// JOBS COMMAND
    }else if(strstr(cmd,"jobs")!=NULL){


// EXIT COMMAND
    }else if(strstr(cmd,"exit\n")!=NULL){
      exit(0);


// CD COMMAND
    }else if(strstr(cmd,"cd")!=NULL){ //
      token = strtok(token, " ");
      if(debug){
        token = strtok(NULL, " ");
      }else{
        token = strtok(cmd, " ");
        token = strtok(NULL, " ");
      }
      token = strtok(token, "\n");
      chdir(token);


// ECHO COMMAND -- NEED TO FIX ""
    }else if(strstr(cmd, "echo")!=NULL){ //
      pid_t pid = fork();
      if (pid == -1){
          // error, failed to fork()
      }else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
      }else if (pid==0){
        token = strtok(NULL," ");
        char str[1024];
        strcpy(str, "");

        while(token != NULL){
          strcat(str, token);
          strcat(str, " ");
          token = strtok(NULL, " ");
        }



        char* echoNQ;
        if(strstr(str,"\"")){
          echoNQ = strtok(str, "\"");
          printf("%s\n",echoNQ);
        }else{
          echoNQ = strtok(str, "\n");
          if(strstr(str,"$")!=0){
            
            char varName[80];
            strcpy(varName, "echo ");
            strcat(varName, str);
            strcat(varName, "\n");
            
            execl(SHELL, SHELL, "-c", varName, NULL);
          }else{
            printf("%s\n",str);
          }
          
        }
        exit(0);
        // execl(SHELL, SHELL, "-c", str, NULL);
      }


// SET COMMAND
    }else if(strstr(cmd, "set")!=NULL){ //
      token = strtok(NULL, " ");
      char* envval = strtok(token, "=");
      char* envname = envval;
      envval = strtok(NULL, "\n");
      setenv(envname, envval, 1);


// REDIRECTION COMMANDS
    }else if(strstr(cmd,">")!=NULL || strstr(cmd,"<")!=NULL){
      pid_t pid = fork();

      if (pid == -1){
          // error, failed to fork()
      }else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
      }else if (pid==0){
        fflush(stdout);
        char* redTok;

        // redirect standard output
        if(strstr(cmd,">")!=NULL){
          // get in and out parts of command
          redTok = strtok(cmd, ">");
          char* left = redTok;
          redTok = strtok(NULL, ">");
          redTok = strtok(redTok, " ");
          redTok = strtok(redTok, "\n");
          char* right = redTok;

          // redirect
          FILE *rfp = fopen(right, "w");
          int rfd = fileno(rfp);
          dup2(rfd, 1);
          execl(SHELL, SHELL, "-c", left, NULL);

        // redirect standard input 
        }else if(strstr(cmd,"<")!=NULL){
          // get left and right parts of redirect
          redTok = strtok(cmd, "<");
          char* left = redTok;
          redTok = strtok(NULL, "<");
          redTok = strtok(redTok, " ");
          redTok = strtok(redTok, "\n");
          char* right = redTok;

          // redirect
          FILE *rfp = fopen(right, "r");
          int rfd = fileno(rfp);
          dup2(rfd, 0);
          execl(SHELL, SHELL, "-c", left, NULL);
        }
      }


// GO HEELS!
    }else if(strcmp(cmd, "goheels\n")==0){
        int c;
        FILE *file;
        file = fopen("goheels.txt", "r");
        if (file) {
          while ((c = getc(file)) != EOF){
            putchar(c);
          }
          fclose(file);
        }


// SYSTEM CALLS
    }else{
      pid_t pid = fork();
      if (pid == -1){
          // error, failed to fork()
      }else if (pid > 0){
          // parent

        int status;
        waitpid(pid, &status, 0);
      }else if (pid==0){
          // child
        if(debug){
          strcpy(tkncmd, token);
          token = strtok(NULL, " ");
          strcat(tkncmd, " ");
          strcat(tkncmd, token);
          printf("%s",tkncmd);
          execl(SHELL, SHELL, "-c", tkncmd, NULL);
        }else{
          execl(SHELL, SHELL, "-c", cmd, NULL);
        }
      }
    }


// DEBUG END
    if(debug){
      token = strtok(tkncmd, " ");
      printf("%s","ENDED: ");
      printf("%s", dbgcmd);
      printf("%s", "ret=");
      fflush(stdout);
      system("echo $?");


      
      debug = false;
    }
    END:
    fflush(stdout);
    argc = 1;
    debug = false;


  }

  return 0;
}