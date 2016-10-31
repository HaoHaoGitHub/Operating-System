# CPU Scheduling Simulator For FCFS and SJF

## Introduction 
In this project, I implemented a rudimentary simulation of an operating system. 

## Conceptual Design
A process is defined as a program in execution. Processes are in one of the following three states: 
(a) ready to use the CPU; (b) actively using the CPU; and (c) blocked on I/O.

Processes in state (a) reside in a simple queue, i.e. the ready queue. This queue is ordered based on 
a configurable CPU scheduling algorithm. In this first assignment, there are two algorithms to implement, 
i.e., first-come-first-serve (FCFS) and shortest job first (SJF). And note that both algorithms will be 
applied to the same set of simulated processes.

Once a process reaches the front of the queue (and the CPU is free to accept a process), the given process 
enters state (b) and executes its current CPU burst.

After the CPU burst is completed, the process enters state (c), performing some sort of I/O operation 
(e.g., writing results to the terminal or a file, interacting with the user, etc.). Once the I/O operation 
completes, the process returns to state (a) and is added to the ready queue 
(its position within the queue is based on the CPU scheduling algorithm)

## Complie 
g++ -Wall *.cpp
./a.out process.txt output.txt
