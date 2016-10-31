/*
   Hao Chen  
   g++ -Wall *.cpp 
   ./a.out process.txt output.txt 
*/
#include "one_process.h"
#include "content.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <list>
#include <queue>
#include <assert.h>
#include <algorithm>
#include <iomanip>

using namespace std;
// Customized_priority_queue
//---------------------------------------------------------------------------
class Foo {
public:
    Foo() {
        pid = 0;
        near_time = 0;
        event_name = " ";
    }

    Foo(int a, int b, string s) {
        pid = a;
        near_time = b;
        event_name = s;
    }

    int pid;
    int near_time;
    string event_name;
};
// Define the operator class for the priority queue:
class Compare {
public: 
    bool operator() (Foo f1, Foo f2) {
        if ((f1.near_time > f2.near_time) || (f1.near_time == f2.near_time && f1.pid > f2.pid)) 
            return true;
        else return false;
    }
};
//---------------------------------------------------------------------------

// Three functions to print out the result:
void print_readyqueue(const list<int>& q);
void print_s_readyqueue(const list<content>& S_ready_queue);
// Useful functions:
void read_input(vector<one_process>& processes, char* r);
void set_context_switch(list<content>& S_ready_queue);
void get_needed_stats(const vector<one_process>& processes, int num, int& index, int& tn, int& tb, int& ti);

int main (int argc, char* argv[]) {
    char* r = argv[1];
    vector<one_process> processes; // vector holds each process
    // Read input 
    // ----------------------------------------
    read_input(processes, r);
    vector<one_process> processes_copy(processes); // Is used for future needs
    // ----------------------------------------
    vector<one_process> S_processes(processes); // Create a copy that will be used in SJF algorithm
    const int n = processes.size(); // # of processes to simulate
    const int t_cs = 9; // Context switch time == 9
    //--------------------------------------------------------------
    // I use the Priority Queue to implement the CPU Scheduling Algorithm:

    // FCFS: 
    // ======================================================================================
    // First, sort by process number: 
    sort(processes.begin(), processes.end(), less_num);
    
    int max_number = 0; // This number is used for setting the size of time_tracking vector
    // Create a list to hold each process number in the ready queue
    list<content> ready_queue;
    for (int i = 0; i < n; ++i) {
        one_process tp = processes[i];
        ready_queue.push_back(content(tp.get_proc_num(), tp.get_burst_time(), 0));
        max_number = max(max_number, tp.get_proc_num());
    }
    
    // If there is a process in cpu, then push that process number into cpu, else cpu vector is 
    // an empty 
    vector<int> cpu;  
    vector<int> time_tracking(max_number,0); // An array to keep track of each process's time
    // Create a priority queue to hold the next events information
    //--------------------------------------------------------------
    priority_queue<Foo, vector<Foo>, Compare> my_pq;
    //--------------------------------------------------------------

    int temp_id = (ready_queue.front()).pid;
    my_pq.push(Foo(temp_id, t_cs, "context switch"));
    time_tracking[temp_id - 1] = t_cs;

    cout << "time 0ms: Simulator started for FCFS ";
    print_s_readyqueue(ready_queue);
    int t = 0; // Keep track of the time in the total processes
    int cpu_burst_time = 0; 
    int wait_time = 0;
    int num_of_context_switch = 0;

    // I use a priority queue to implement the whole events: All the events can be divided 
    // into three categories: 1. context switch. 2. complete cpu. 3 complete IO. Get the 
    // nearest event when iterating in the while loop.

    while (!my_pq.empty()) {
        
         Foo temp = my_pq.top(); // Get the next nearest event 
         t = temp.near_time;
         int index, temp_num_burst, temp_burst_time, temp_IO_time;

         if (temp.event_name == "context switch") {
            int num = ready_queue.front().pid;
            //get useful statistics
            get_needed_stats(processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);

            if (temp_num_burst > 0) {
                if (!ready_queue.empty() && cpu.empty()) { 
                   temp_num_burst --;
                   processes[index].set_n(temp_num_burst);
                   cout << "time " << t << "ms: " << "P" << temp.pid << " " << "started using the CPU ";
                   cpu.push_back(num);
                   // -----------------------------------------------------
                   wait_time += t - processes[index].get_wait_time();
                   processes[index].set_w(0);
                   // -----------------------------------------------------

                   ready_queue.pop_front();
                   print_s_readyqueue(ready_queue);
            
                   my_pq.pop();
                   time_tracking[num - 1] = t + temp_burst_time;
                   my_pq.push(Foo(num, time_tracking[num - 1], "complete cpu"));
                } 
            } 

         } else if (temp.event_name == "complete cpu") {
            int num = temp.pid;
            get_needed_stats(processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);
            // ----------------------------------------------------------------------
            if (temp_num_burst != 0) {
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "completed its CPU burst ";
                print_s_readyqueue(ready_queue);
                if (temp_IO_time != 0) {
                    cout << "time " << t << "ms: " << "P" << temp.pid << " " << "blocked on I/O ";
                    print_s_readyqueue(ready_queue);
                } 
            } else {
                assert(temp_num_burst == 0);
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "terminated ";
                print_s_readyqueue(ready_queue);
                if (cpu.empty() && ready_queue.size() == 1) { 
                   set_context_switch(ready_queue);
                   my_pq.push(Foo(ready_queue.front().pid, t + t_cs, "context switch"));
                }
            }
            cpu.clear();
            // ----------------------------------------------------------------------
            
            my_pq.pop();
            if (!ready_queue.empty() && cpu.empty()) {
                set_context_switch(ready_queue);
                my_pq.push(Foo(ready_queue.front().pid, t + t_cs, "context switch"));
            } 

            if (temp_IO_time != 0 && temp_num_burst != 0) {
                 my_pq.push(Foo(num, t + temp_IO_time, "complete IO"));
            }

         } else if (temp.event_name == "complete IO") {
            int num = temp.pid; 
            //set parameter
            get_needed_stats(processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);

            if (temp_num_burst > 0) {
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "completed I/O ";
                ready_queue.push_back(content(num, temp_burst_time, 0));
                print_s_readyqueue(ready_queue);
                // -----------------------------------------------------
                processes[index].set_w(t); 
                // -----------------------------------------------------
                // If ready queue used to be empty!!!!!!!!!!!! and cpu is empty......
                if (cpu.empty() && ready_queue.size() == 1) {
                   set_context_switch(ready_queue); 
                   my_pq.push(Foo(ready_queue.front().pid, t + t_cs, "context switch"));
                }
            } 
            my_pq.pop();
         }
    }
    cout << "time " << t << "ms: Simulator ended for FCFS" << endl;
    cout << endl;
    
    // Print out statistics for each simulated algorithm:
    ofstream out_str(argv[2]);
    if (!out_str.good()) {
        std::cerr << "Can't open " << argv[2] << " to write.\n";
        exit(1);
    }
    out_str << "Algorithm FCFS" << endl;
    int total_num_of_process = 0;
    for (unsigned int i = 0; i < processes_copy.size(); ++i) {
        cpu_burst_time += (processes_copy[i].get_burst_time())*(processes_copy[i].get_num_burst());
        total_num_of_process += processes_copy[i].get_num_burst();
    }

    float averg_cpu = float(cpu_burst_time)/total_num_of_process;
    float averg_wait = float(wait_time)/total_num_of_process - t_cs;
    float averg_turn = averg_cpu + averg_wait + t_cs;
    num_of_context_switch = total_num_of_process;

    out_str << "-- average CPU burst time: " << fixed << setprecision(2) << averg_cpu << " ms" << endl;
    out_str << "-- average wait time: " << fixed << setprecision(2) << averg_wait << " ms" << endl;
    out_str << "-- average turnaround time: " << fixed << setprecision(2) << averg_turn << " ms" << endl;
    out_str << "-- total number of context switches: " << num_of_context_switch << endl;


    // ======================================================================================
    // SJF 
    // ======================================================================================
    // don't need to sort the processes
    // Create a priority queue to hold each process number in the ready queue
    list<content> S_ready_queue;
    for (int i = 0; i < n; ++i) {
        one_process tp = S_processes[i];
        S_ready_queue.push_back(content(tp.get_proc_num(), tp.get_burst_time(), 0));
    }
    
    cpu.clear();

    // Sort the ready queue based on certain rules:
    S_ready_queue.sort(nearer);
    // An array to keep track of each process's time separately!!
    vector<int> S_time_tracking(max_number, 0);  
    // Create a priority queue to hold the next events information
    //--------------------------------------------------------------
    priority_queue<Foo, vector<Foo>, Compare> S_my_pq;
    //--------------------------------------------------------------
    
    temp_id = (S_ready_queue.front()).pid;
    S_my_pq.push(Foo(temp_id, t_cs, "context switch"));
    S_time_tracking[temp_id - 1] = t_cs;

    cout << "time 0ms: Simulator started for SJF ";
    print_s_readyqueue(S_ready_queue);
    t = 0; // Reset elapsed time t back to 0
    wait_time = 0;
    
    while (!S_my_pq.empty()) {
        
        Foo temp = S_my_pq.top();
        t = temp.near_time;
        S_ready_queue.sort(nearer);
        int index, temp_num_burst, temp_burst_time, temp_IO_time;

        if (temp.event_name == "context switch") {
            int num = (S_ready_queue.front()).pid;
            get_needed_stats(S_processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);

            if (temp_num_burst > 0) {
                if (!S_ready_queue.empty() && cpu.empty()) { 
                   temp_num_burst --;
                   S_processes[index].set_n(temp_num_burst);
                   cout << "time " << t << "ms: " << "P" << num << " " << "started using the CPU ";
                   cpu.push_back(num);
                   // -----------------------------------------------------
                   wait_time += t - S_processes[index].get_wait_time();
                   S_processes[index].set_w(0);
                   // -----------------------------------------------------

                   S_ready_queue.pop_front();
                   print_s_readyqueue(S_ready_queue);
            
                   S_my_pq.pop();
                   S_time_tracking[num - 1] = t + temp_burst_time;
                   S_my_pq.push(Foo(num, S_time_tracking[num - 1], "complete cpu"));
                } 
            } 

        } else if (temp.event_name == "complete cpu") {
            int num = temp.pid;
            get_needed_stats(S_processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);

            // ----------------------------------------------------------------------
            if (temp_num_burst != 0) {
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "completed its CPU burst ";
                print_s_readyqueue(S_ready_queue);
                if (temp_IO_time != 0) {
                    cout << "time " << t << "ms: " << "P" << temp.pid << " " << "blocked on I/O ";
                    print_s_readyqueue(S_ready_queue);
                } 
            } else {
                assert(temp_num_burst == 0);
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "terminated ";
                print_s_readyqueue(S_ready_queue);
                if (cpu.empty() && S_ready_queue.size() == 1) { 
                   //------------------------------------
                   set_context_switch(S_ready_queue);
                   //------------------------------------
                   S_my_pq.push(Foo((S_ready_queue.front()).pid, t + t_cs, "context switch"));
                }
            }
            cpu.clear();
            // ----------------------------------------------------------------------
            S_my_pq.pop();

            if (!S_ready_queue.empty() && cpu.empty()) {
                S_ready_queue.sort(nearer);
                // ----------------------------------
                set_context_switch(S_ready_queue);
                // ----------------------------------
                S_my_pq.push(Foo((S_ready_queue.front()).pid, t + t_cs, "context switch"));
            }

            if (temp_IO_time != 0 && temp_num_burst != 0) {
                S_my_pq.push(Foo(num, t + temp_IO_time, "complete IO"));
            }

        } else if (temp.event_name == "complete IO") {
            int num = temp.pid; 
            get_needed_stats(S_processes, num, index, temp_num_burst, temp_burst_time, temp_IO_time);

            if (temp_num_burst > 0) {
                cout << "time " << t << "ms: " << "P" << temp.pid << " " << "completed I/O "; 
                S_ready_queue.push_back(content(num, temp_burst_time, 0));
                S_ready_queue.sort(nearer);
                print_s_readyqueue(S_ready_queue);
                // -----------------------------------------------------
                S_processes[index].set_w(t); 
                // -----------------------------------------------------
                // If ready queue used to be empty!!!!!!!!!!!! and cpu is empty......
                if (cpu.empty() && S_ready_queue.size() == 1) { 
                   S_ready_queue.sort(nearer);
                   //----------------------------------
                   set_context_switch(S_ready_queue);
                   //----------------------------------
                   S_my_pq.push(Foo((S_ready_queue.front()).pid, t + t_cs, "context switch"));
                }
            }
            S_my_pq.pop();
        }
    } 
    cout << "time " << t << "ms: Simulator ended for SJF" << endl;

    out_str << "Algorithm SJF" << endl;
    averg_wait = float(wait_time)/total_num_of_process - t_cs;
    averg_turn = averg_cpu + averg_wait + t_cs;
    out_str << "-- average CPU burst time: " << fixed << setprecision(2) << averg_cpu << " ms" << endl;
    out_str << "-- average wait time: " << fixed << setprecision(2) << averg_wait << " ms" << endl;
    out_str << "-- average turnaround time: " << fixed << setprecision(2) << averg_turn << " ms" << endl;
    out_str << "-- total number of context switches: " << num_of_context_switch << endl;
}

void get_needed_stats(const vector<one_process>& processes, int num, int& index, int& tn, int& tb, int& ti) {
    for (int i = 0; i < (int)processes.size(); ++i) {
        if (processes[i].get_proc_num() == num) {
            tn = processes[i].get_num_burst(); 
            tb = processes[i].get_burst_time();
            ti = processes[i].get_io_time();
            index = i;
            break;
        }
    }
}

void set_context_switch(list<content>& S_ready_queue) {
    if (S_ready_queue.front().get_context_switch() == 1) {
        S_ready_queue.front().set_cs(2);
    } else {
        if (S_ready_queue.front().get_context_switch() == 0) {
            S_ready_queue.front().set_cs(1);
        }
    }
}


void print_processes(const vector<one_process>& processes){
    // Print out the details:
    for(int i = 0; i < (int)processes.size(); ++i) {
        cout << processes[i].get_proc_num() << " "<< processes[i].get_burst_time() << " "
             <<  processes[i].get_num_burst() << " " << processes[i].get_io_time() << endl;
    }
}

void print_s_readyqueue(const list<content>& S_ready_queue) {
    // Corner case 
    if (S_ready_queue.size() == 0) { cout << "[Q]" << endl; return;}
    int count = 0;
    int n = S_ready_queue.size();
    cout << "[Q";
    
    for (list<content>::const_iterator i = S_ready_queue.begin(); i != S_ready_queue.end(); ++i) {
        if (i->get_context_switch() == 1) continue;
        if (count != n - 1) {
            cout << " " << i->pid;
        } else {
            cout << " " << i->pid;
        }
        count ++;
    }
    cout << "]";
    cout << endl;
}

void read_input(vector<one_process>& processes, char* r) {
    ifstream in_str(r);

    if (!in_str.good()) {
        std::cerr << "Can't open the grades file" << endl;
        return;
    }

    if (!in_str.is_open()) {
        cout << "Not successfully opened" << endl;
        return;
    }
    string line;
    while (getline(in_str, line)) {
        if (line[0] == '#') continue;
        if (line.empty()) continue; // Test for corner cases:

        int start = 0; int end = 0; 
        vector<int> v1; // hold 4 indexes in one line
        for (unsigned int i = 0; i < line.size(); ++i) {
             if (line[i] != '|' && i != line.size() - 1) {
                end ++;
             } else {
                if (i == line.size() - 1) end++;
                string temp_s;
                for (int m = start; m < end; m ++) {temp_s += line[m];}
                int n = atoi(temp_s.c_str()); // Convert string to int 
                v1.push_back(n);
                end ++;
                start = end;
             }  
        }
        one_process temp_p(v1[0], v1[1], v1[2], v1[3], 0);
        processes.push_back(temp_p);
    }
} 







