# Inter-Process Communication (IPC) using Pipes in C
## Introduction 
I use C to implement a fork-based calculator program. 
Overall, my progrm reads an input file that specifies a set of calculations to be made using a Scheme/LISP-like format.
More specifically, my program will call fork() to create child processes to perform each calculation, thus constructing
a process tree that represents the fully parsed mathematical expression. 

## Structure

              10         (- 10 4)        18

    PARENT:  pipe()
             fork()                                    <=== 1 child process
                          pipe()
                          fork()                       <=== 2 children
                                       pipe()
                                       fork()          <=== 3 children

             read()
             waitpid()                                 <=== 2 children left
                          read()
                          waitpid()                    <=== 1 child left
                                       read()
                                       waitpid()       <=== 0 children/zombies

             /* we have the result for the given expression */
                                     
      process_expr( "(* 10 (- 10 4) 18)", i, j )    <=== happens in the parent
                     i                j


       ...... process_expr( "(- 10 4)", i, j )     <=== happens in the child
                             i      j
