#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int status;
  pid_t pid_1, pid_2, pid_3, pid_4;
  
  int p1[2], p2[2], p3[2];

  //Checking if input parameters DIR, STR are correct
  if (argc != 4) {
    printf("usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }else{
    printf("I'm meant to be here\n");//don't delete this print statement
  }

  //STEP 1
	//Initialize pipes p1, p2, and p3

  pipe(p1); // Initializing pipe 1
  pipe(p2); // Initializing pipe 2
  pipe(p3); // Initializing pipe 3
  // We want to initialize our pipes before forking so that the child can access to the same file descripters as the parent,
  // thus, being able to communicate. -Anna

  char cmdbuf[BSIZE]; // Create a buffer cmdbuf to hold the command string, with size BSIZE. -Anna

  pid_1 = fork();
  if (pid_1 == 0) {
    /* First Child */

    //STEP 2
		//In this first child process, we want to send everything that is printed on the standard output, to the next child process through pipe p1
		//So, redirect standard output of this child process to p1's write end - written data will be automatically available at pipe p1's read end
		//And, close all other pipe ends except the ones used to redirect the above OUTPUT (very important)

    bzero(cmdbuf, BSIZE); // Clear the memory of the buffer array by setting all bytes (the size of BSIZE) to zero. -Anna
    
    sprintf(cmdbuf, "%s %s -name \'*\'.[h]", FIND_EXEC, argv[1]); // Construct the command string that 
                                                                  // searches for all files in the specified directory
                                                                  // defined by argv[1]. FIND_EXEC is the path to the find command. -Anna

    dup2(p1[1], STDOUT_FILENO); // Redirect the output to the write end of pipe p1, which is p1[1]. -Anna
    close(p1[0]); // Close the read end of pipe p1 because we are not using it. -Anna
    close(p2[0]); // Close the read end of pipe p2 because we are not using it. -Anna
    close(p2[1]); // Close the write end of pipe p2 because we are not using it. -Anna
    close(p3[0]); // Close the read end of pipe p3 because we are not using it. -Anna
    close(p3[1]); // Close the write end of pipe p3 because we are not using it. -Anna
    //STEP 3
    //Prepare a command string representing the find command (follow example from the slide)
    //Invoke execl for bash and find (use BASH_EXEC and FIND_EXEC as paths)

    // execl function replaces the current process with a new process. -Anna
    // It is executing BASH_EXEC with the c flag, which tells Bash to execute the command stored in cmdbuff. -Anna
    // (char*) 0 is a null pointer that marks the end of the argument list for execl. -Anna
    // If execl fails (meaning there is too many processes running), return and handle the error. -Anna
    if ((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0)) < 0) {
      fprintf(stderr, "\nError execing find. ERROR#%d\n", errno); // Error message.
    }

    exit(0);
  }

  pid_2 = fork();
  if (pid_2 == 0) {
    /* Second Child */

    //STEP 4
    //In this second child process, we want to receive everything that is available at pipe p1's read end, and use the received information as standard input for this child process
		//In this second child process, we want to send everything that is printed on the standard output, to the next child process through pipe p2
		//So, redirect standard output of this child process to p2's write end - written data will be automatically available at pipe p2's read end
		//And, close all other pipe ends except the ones used to redirect the above two INPUT/OUTPUT (very important)

    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]);

    dup2(p1[0], STDIN_FILENO); // Redirect the input to the read end of pipe p1, which is p1[0]. -Anna
    dup2(p2[1], STDOUT_FILENO); // Redirect the output to the write end of pipe p2, which is p2[1]. -Anna

    close(p1[1]); // Close the write end of pipe p1 because we are not using it. -Anna
    close(p2[0]); // Close the read end of pipe p2 because we are not using it. -Anna
    close(p3[0]); // Close the read end of pipe p3 because we are not using it. -Anna
    close(p3[1]); // Close the write end of pipe p3 because we are not using it. -Anna

    //STEP 5
    //Invoke execl for xargs and grep (use XARGS_EXEC and GREP_EXEC as paths)

    if ((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0)) < 0) {
      fprintf(stderr, "\nError execing xargs grep. ERROR#%d\n", errno);
    }

    exit(0);
  }

  pid_3 = fork();
  if (pid_3 == 0) {
    /* Third Child */

    //STEP 6
		//In this third child process, we want to receive everything that is available at pipe p2's read end, and use the received information as standard input for this child process
		//In this third child process, we want to send everything that is printed on the standard output, to the next child process through pipe p3
		//So, redirect standard output of this child process to p3's write end - written data will be automatically available at pipe p3's read end
		//And, close all other pipe ends except the ones used to redirect the above two INPUT/OUTPUT (very important)

    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC);

    dup2(p2[0], STDIN_FILENO); // Redirect the input to the read end of pipe p2, which is p2[0]. -Anna
    dup2(p3[1], STDOUT_FILENO); // Redirect the output to the write end of pipe p3, which is p3[1]. -Anna

    close(p2[1]); // Close the write end of pipe p2 because we are not using it. -Anna
    close(p3[0]); // Close the read end of pipe p3 because we are not using it. -Anna
    close(p1[0]); // Close the read end of pipe p1 because we are not using it. -Anna
    close(p1[1]); // Close the write end of pipe p1 because we are not using it. -Anna

    //STEP 7
    //Invoke execl for sort (use SORT_EXEC as path)

    if ((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0)) < 0) {
      fprintf(stderr, "\nError execing sort. ERROR#%d\n", errno);
    }

    exit(0);
  }

  pid_4 = fork();
  if (pid_4 == 0) {
    /* Fourth Child */

    //STEP 8
		//In this fourth child process, we want to receive everything that is available at pipe p3's read end, and use the received information as standard input for this child process
		//Output of this child process should directly be to the standard output and NOT to any pipe
		//And, close all other pipe ends except the ones used to redirect the above INPUT (very important)

    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s --line=%s", HEAD_EXEC, argv[3]);
    
    dup2(p3[0], STDIN_FILENO); // Redirect the input to the read end of pipe p3, which is p3[0]. -Anna

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    close(p3[1]);

    //STEP 8
    //Invoke execl for head (use HEAD_EXEC as path). Print only the first 5 results in the output

    if ((execl(HEAD_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0)) < 0) {
      fprintf(stderr, "\nError execing sort. ERROR#%d\n", errno);
    }

    exit(0);
  }

  close(p1[0]); // Close the read end of pipe p1 because we are not using it. -Anna
  close(p1[1]); // Close the write end of pipe p1 because we are not using it. -Anna
  close(p2[0]); // Close the read end of pipe p2 because we are not using it. -Anna
  close(p2[1]); // Close the write end of pipe p2 because we are not using it. -Anna
  close(p3[0]); // Close the read end of pipe p3 because we are not using it. -Anna
  close(p3[1]); // Close the write end of pipe p3 because we are not using it. -Anna

  if ((waitpid(pid_1, &status, 0)) == -1) {
    fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_2, &status, 0)) == -1) {
    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_3, &status, 0)) == -1) {
    fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_4, &status, 0)) == -1) {
    fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }

  return 0;
}
