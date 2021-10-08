/***
 * @file jl_processes.cpp
 * @author Joseph Lan
 *         Original from UWB CSS 430 Fall 2021
 * 
 * @brief This program runs the equivalent linux command:
 *          ps -A | grep arg | wc -l
 *        where arg is a command line input argument into the program
 *        
 * @version 0.1
 * @date 2021-10-07
 * 
 * @copyright Copyright (c) 2021
 * 
 * CSS 430 Operating Systems
 * Prof. Robert Palmer
 * Fall 2021
 */
#include <sys/types.h>   // for fork, wait
#include <sys/wait.h>    // for wait
#include <unistd.h>      // for fork, pipe, dup, close
#include <stdio.h>       // for NULL, perror
#include <stdlib.h>      // for exit
#include <iostream>      // for cout

using namespace std;

/**
 * @brief Program driver creates child processes and pipes to run the equivalent
 *        linux command ps -A | grep arg | wc -l
 * 
 * @pre requires command line input of a string to look for in processes
 * 
 * @param argc number of input arguments
 * @param argv input character array. [1] holds the string to look for
 * @return int program execution status code
 */
int main( int argc, char** argv ) {

  int fds[2][2]; // pipe
  int pid; // process identifier

  // If not two arguments, program was run missing an argument.
  if (argc != 2) {
    cerr << "No argument was provided to the program." << endl;
    exit(-1);
  }

  // Fork to create a child process
  if ((pid = fork()) < 0 ) { // less than 0, fork fails
    perror("Fork error creating child.");

  // if pid == 0 process is child, otherwise process is parent
  } else if (pid == 0) {
    // I'm the child
    
    // Create a pipe using fds[0]
    // fds[0] pipe will be used for grep ssh | wc -l
    if (pipe(fds[0]) < 0) { // less than 0, pipe fails
      cerr << "Pipe creation at fds[0] failed." << endl;
      exit(-1);
    }

    int pid_child; // New pid identifier in child process

    // Fork a grand-child process
    if ((pid_child = fork()) < 0) { // less than 0, fork fails
      perror("Fork error creating grand child.");

    // If pid == 0 process is grand child, otherwise process is child
    } else if (pid_child == 0) {
      // I'm the grand child
      
      // create a pipe using fds[1]
      // fds[1] pipe will be used for grep ssh | wc -l
      if (pipe(fds[1]) < 0) { // less than 0, pipe fails
        cerr << "Pipe creation at fds[1] failed." << endl;
        exit(-1);
      }
      int pid_gchild; // New pid identifier in grand child process

	    // Fork a great-grand-child
      // If less than 0, fork failed.
      if ((pid_gchild = fork()) < 0) {
        perror("Fork error creating great grand child.");

      // If pid == 0 process is great grand child, otherwise grand child
      } else if (pid_gchild == 0) {
        // I'm the great grand child, I run ps -A
        
        // duplicate fds[1][1] to standard output
        dup2(fds[1][1], 1);
        close(fds[1][1]);
        close(fds[1][0]);
        close(fds[0][1]);
        close(fds[0][0]);

        // Execute ps -A and write contents to fds[0][1]
        execlp("ps", "ps", "-A", (char *)NULL);
      } else {
        // I'm the grand child, i will execute grep argv[1]

        // duplicate fds[1][1] to standard input
        dup2(fds[1][0],0);
        close(fds[1][0]);
        close(fds[1][1]);

        // duplicate fds[0][1] to standard output
        dup2(fds[0][1], 1);
        close(fds[0][1]);
        close(fds[0][0]);

        // Execute grep argv[1] and write contents to fds[1][1]
        execlp("grep", "grep", argv[1],(char *)NULL);
      }
    } else {
      // I'm the child, I will execute wc -l

      // duplicate fds[1][0] to standard input
      dup2(fds[0][0],0);
      close(fds[0][0]);
      close(fds[0][1]);

      // Child executes wc -1
      // execute wc -l
      execlp("wc", "wc", "-l", (char *)NULL);
    }
  } else { // I am the parent
    wait(NULL); // wait for child process to complete
  }
  return 0;
}