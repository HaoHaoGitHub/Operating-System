/* Hao Chen  661412823  chenh15@rpi.edu */

#include <stdlib.h>     /* atoi */
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <algorithm>
#include "one_process.h"


using namespace std;
// Initilizes variables
int t; // Global variable to keep track of the timing process 
const int t_cs = 9; // Context time
const int t_slice = 80; // default value of time slice

// Customized_priority_queue
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
        if ((f1.near_time > f2.near_time) || 
            (f1.near_time == f2.near_time && f1.pid > f2.pid)) 
            return true;
        else return false;
    }
};



class Another_Foo;

typedef pair<int, one_process*> customized_pair;
typedef map<int, pair<int, vector<one_process*> > > customized_map;
typedef priority_queue<customized_pair, vector<customized_pair>, Another_Foo > My_Priority;

// Define the operator class for the priority queue:
class Another_Foo {
    bool index;
public:
    Another_Foo(const bool& r = false) {index = r;}
    bool operator() (const customized_pair& p1, const customized_pair& p2) const {
        if (p1.second->context_switch != p2.second->context_switch) {
            if (p1.second->context_switch < p2.second->context_switch) return true;
            else return false;
        } else if (p1.second->preempt_other != p2.second->preempt_other) {
            if (p1.second->preempt_other < p2.second->preempt_other) return true;
            else return false;
        }  else if (p1.first != p2.first) {
            if (p1.first > p2.first) return true;
            else return false;
        } else {
            if (p1.second->pid > p2.second->pid) return true;
            else return false;
        }
    }   
};

// Useful functions:
void read_input(vector<one_process> &processes, char* r);
bool compare_functor(const one_process& p1, const one_process& p2);
void print_queue(queue<one_process*> q);
void print_priority_queue(My_Priority pq);
void print_given_queue(const string& name_of_algo, 
                       queue<one_process*>& fcfs_queue, 
                       My_Priority& srt_queue, 
                       queue<one_process*>& rr_queue);
bool is_ended(const vector<one_process>& processes);
void push_event(vector<one_process>& processes, int t, 
                const string& name_of_algo, 
                queue<one_process*>& fcfs_queue, 
                My_Priority& srt_queue, 
                queue<one_process*>& rr_queue, 
                vector<customized_pair>& processes_in_tie, 
                customized_map& event_map);
void pop_event(vector<one_process>& processes, 
               const string& name_of_algo, 
               queue<one_process*>& fcfs_queue, 
               My_Priority& srt_queue, 
               queue<one_process*>& rr_queue, 
               vector<customized_pair>& processes_in_tie, 
               customized_pair& c_process, 
               customized_map& event_map);
bool compare_functor(const one_process& p1, const one_process& p2);
void slice_adjust_push_event(customized_map& event_map, int& t, one_process* p);
void print_useful_statistcs(ostream& ostr, 
                            const vector<one_process>& processes, 
                            const string& name_of_algo);
void run_different_algorthms(vector<one_process>& processes, 
                             const string& name_of_algo, 
                             customized_map& event_map, 
                             vector<customized_pair>& processes_in_tie, 
                             customized_pair& c_process, 
                             queue<one_process*>& fcfs_queue, 
                             My_Priority& srt_queue, 
                             queue<one_process*>& rr_queue, 
                             ofstream& ostr);



/* =====================================================================*/
/* =====================================================================*/
int main(int argc, char* argv[]) {

    /* Create useful variables: */
    // -----------------------------------------------------
    vector<one_process> processes;  // vector of input processes
    queue<one_process*> fcfs_queue, rr_queue;
    My_Priority srt_queue; 
    customized_map event_map; // map containing different events
    vector<customized_pair> processes_in_tie; // vector to store tie processes
    customized_pair c_process; // the process that is running in CPU
    c_process.first = 0; c_process.second = NULL;  // initialize the process in cpu
    // -----------------------------------------------------
    
    /* Input and Output */
    // Read input
    // -----------------------------------------------------
    char* r = argv[1];
    read_input(processes,r);
    // -----------------------------------------------------
    ofstream ostr;
    ostr.open("simout.txt");

    // First Come First Serve: 
    run_different_algorthms(processes, "FCFS", event_map, processes_in_tie, c_process, 
                            fcfs_queue, srt_queue, rr_queue, ostr);
    // Shortest_job_remaining (preemptive):
    run_different_algorthms(processes, "SRT", event_map, processes_in_tie, c_process, 
                            fcfs_queue, srt_queue, rr_queue, ostr);
    // Round Robin:
    run_different_algorthms(processes, "RR", event_map, processes_in_tie, c_process, 
                            fcfs_queue, srt_queue, rr_queue, ostr);
    
    return 0;
}

/* =====================================================================*/
/* =====================================================================*/

// read input helper function 
void read_input(vector<one_process> &processes, char* r){
    ifstream in_str(r);
    string line;
    if (!in_str.good()) {
        std::cerr << "Can't open the grades file" << endl;
        return;
    }

    if (!in_str.is_open()) {
        cout << "Not successfully opened" << endl;
        return;
    }

    while (getline(in_str, line)) {
        if (line[0] == '#') continue;
        if (line.empty()) continue; // Test for corner cases:

        int start = 0; int end = 0; 
        vector<int> v1; // hold 5 indexes in one line
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
        one_process temp_p(v1[0], v1[1], v1[2], v1[3], v1[4]);
        processes.push_back(temp_p);
    }
}


void run_different_algorthms(vector<one_process>& processes, 
                             const string& name_of_algo, 
                            customized_map& event_map, 
                            vector<customized_pair>& processes_in_tie, 
                            customized_pair& c_process, 
                            queue<one_process*>& fcfs_queue, 
                            My_Priority& srt_queue, 
                            queue<one_process*>& rr_queue, 
                            ofstream& ostr) {
    // Run different simulation
    cout << "time 0ms: Simulator started for " << name_of_algo;
    // If current algoritm is Round Robin: 
    if (name_of_algo == "RR") {
        cout << " (t_slice is " << t_slice << "ms)";
    }
    cout << endl;

    // set_original data
    // -----------------------------------------------------------------------------------
    for (unsigned int i = 0; i < processes.size(); ++i) { processes[i].set_original(); }
    sort(processes.begin(), processes.end(), compare_functor);
    // -----------------------------------------------------------------------------------

    push_event(processes, 0, name_of_algo, fcfs_queue, srt_queue, rr_queue, 
               processes_in_tie, event_map);
    
    while (!is_ended(processes)) {
        pop_event(processes, name_of_algo, fcfs_queue, srt_queue, rr_queue, 
                  processes_in_tie, c_process, event_map);
    }
    
    cout << "time " << t << "ms: Simulator ended for " << name_of_algo;
    cout << endl;
    if (name_of_algo != "RR") {
        cout << endl;
    }
    // Output usefull statistics
    print_useful_statistcs(ostr, processes, name_of_algo);
}


// Print queue
void print_queue(queue<one_process*> q) {
    cout << "[Q";
    while (!q.empty()) { cout << " " << q.front()->pid; q.pop();}
    cout << "]" << endl;
}

// Print priority queue
void print_priority_queue(My_Priority pq) {
    cout << "[Q";
    while (!pq.empty()) {
        if (pq.top().second->preempt_other == false && 
            pq.top().second->context_switch == false) {
            cout << " " << pq.top().second->pid; pq.pop();
        }
    }
    cout << "]" << endl;
}

// Print corresponding queues:
void print_given_queue(const string& name_of_algo, 
                       queue<one_process*>& fcfs_queue, 
                       My_Priority& srt_queue, 
                       queue<one_process*>& rr_queue) {
    if (name_of_algo == "SRT") print_priority_queue(srt_queue);
    if (name_of_algo == "RR") print_queue(rr_queue);
    if (name_of_algo == "FCFS") print_queue(fcfs_queue);
}



// Check whether the simulation is the last simulation process: 
bool is_ended(const vector<one_process>& processes) {
    for (unsigned int i = 0; i < processes.size(); ++i) { 
        if (processes[i].left_num_burst != 0) return false;
    }
    return true;
}



void push_event(vector<one_process>& processes, 
                int t, const string& name_of_algo, 
                queue<one_process*>& fcfs_queue, 
                My_Priority& srt_queue, 
                queue<one_process*>& rr_queue, 
                vector<customized_pair>& processes_in_tie,
                customized_map& event_map) {
    
    // Check whether the queue is empty
    bool is_empty;
    
    if (name_of_algo == "FCFS") {
        is_empty = fcfs_queue.empty();
    } else if (name_of_algo == "SRT") {
        is_empty = srt_queue.empty();
    } else {
        is_empty = rr_queue.empty();
    }
   
    // Put all processes into event map based on arrival time, at time 0
    if (t == 0 && is_empty) {
        customized_map::iterator itr;
        for (unsigned int i = 0; i < processes.size(); ++i) {
            
            int t_arrive = processes[i].arrival_time; 
            one_process* c_process = &processes[i];
            
            if (event_map.find(t_arrive) != event_map.end()) {
                event_map.find(t_arrive)->second.second.push_back(c_process);
            }      
            else {
                vector<one_process*> tmp_v;
                tmp_v.push_back(c_process);
                event_map.insert(make_pair(t_arrive, make_pair(0, tmp_v)));
            }
        }
    } else {
        // Get the nearest process in the ready queue:
        if (is_empty) return;

        vector<one_process*> temp_vector;
        one_process* c_process;
        if (name_of_algo == "FCFS") {
            c_process = fcfs_queue.front();
        } else if (name_of_algo == "SRT") {
            c_process = srt_queue.top().second;
        } else {
            c_process = rr_queue.front();
        }

        temp_vector.push_back(c_process);
        
        // caculate related event time
        int temp_burst_start = t + t_cs;
        int temp_burst_end;
        if (c_process->preempting) {
            temp_burst_end = temp_burst_start + c_process->left_burst_time;
            c_process->preempting = false;
        } else { temp_burst_end = temp_burst_start + c_process->burst_time;}
        int next_IO_time = temp_burst_end + c_process->io_time;
        
        pair<customized_map::iterator, bool> new_r;
        //---------------------------------------------------------------------------------
        new_r = event_map.insert(make_pair(temp_burst_start, make_pair(1, temp_vector)));
        new_r = event_map.insert(make_pair(temp_burst_end, make_pair(2, temp_vector)));
        //---------------------------------------------------------------------------------
        if (c_process->io_time == 0 || c_process->left_num_burst == 1) return;
        new_r = event_map.insert(make_pair(next_IO_time, make_pair(3, temp_vector)));

        if (new_r.second == false) {
            processes_in_tie.push_back(make_pair(next_IO_time, c_process));
        }
    }
}



void pop_event(vector<one_process>& processes, 
               const string& name_of_algo, 
               queue<one_process*>& fcfs_queue, 
               My_Priority& srt_queue, 
               queue<one_process*>& rr_queue, 
               vector<customized_pair>& processes_in_tie, 
               customized_pair& c_process,
               customized_map& event_map) {
    
    // First, get the nearest event in the even map to have the following analyses:
    customized_map::iterator it = event_map.begin();
    
    // A process start cpu burst 
    if (it->second.first == 1) {
        if (name_of_algo == "FCFS") {
            c_process = make_pair(1, fcfs_queue.front());
            fcfs_queue.pop();
        }
        if (name_of_algo == "SRT") {
            c_process = make_pair(1, srt_queue.top().second);
            if (srt_queue.top().second->preempt_other == true) {
                srt_queue.top().second->preempt_other = false;
            }
            srt_queue.pop();
        } else if (name_of_algo == "RR") {
            c_process = make_pair(1, rr_queue.front());
            rr_queue.pop();
            int burst_time = it->second.second[0]->left_burst_time;
            if (burst_time > t_slice) {
                int new_t = it->first + t_slice;
                slice_adjust_push_event(event_map, new_t, it->second.second[0]);
            }
        }
        
        // Waiting time & CPU time
        int curr_wait_t = it->first - it->second.second[0]->time_enterqueue - t_cs;
        it->second.second[0]->total_wt += curr_wait_t;
        it->second.second[0]->time_entercpu = it->first;
        cout << "time " << it->first << "ms: P" 
             << it->second.second[0]->pid << " started using the CPU ";
        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
    }
    
    // If an event: block on IO
    if (it->second.first == 2) {
        --(it->second.second[0]->left_num_burst);
        c_process.first = 0;
        c_process.second = NULL;
        it->second.second[0]->left_burst_time = it->second.second[0]->burst_time;
        if (!(it->second.second[0]->left_num_burst)) {
            cout << "time " << it->first << "ms: P" 
                 << it->second.second[0]->pid << " terminated ";
            print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
        } else {
            cout << "time " << it->first << "ms: P"<< it->second.second[0]->pid 
                 << " completed its CPU burst ";
            print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
            cout << "time " << it->first << "ms: P"<< it->second.second[0]->pid 
                 << " blocked on I/O ";
            print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
        }
        push_event(processes, 
                   it->first, 
                   name_of_algo, 
                   fcfs_queue, 
                   srt_queue, 
                   rr_queue, 
                   processes_in_tie, 
                   event_map);
    }

    // Complete IO 
    if (it->second.first == 3) {
        
        cout << "time " << it->first << "ms: P"<< it->second.second[0]->pid 
             << " completed I/O ";
        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
        
        bool q_empty;
        if (name_of_algo == "FCFS") {
            q_empty = fcfs_queue.empty();
        } else if (name_of_algo == "SRT") {
            q_empty = srt_queue.empty();
        } else {
            q_empty = rr_queue.empty();
        }
        
        if (processes_in_tie.empty()) {
            bool check_preempting = false;
            
            if (name_of_algo == "SRT") { if (c_process.first == 0) { if (q_empty == false) {
                //the top of srt_queue is in context switch
                srt_queue.top().second->context_switch = true;
                c_process.second = srt_queue.top().second;
                if (c_process.second->left_burst_time > it->second.second[0]->left_burst_time) 
                    check_preempting = true;
                    // if there is no preempt
                    if (check_preempting == false) {
                    for (customized_map::iterator itr = event_map.begin(); 
                         itr != event_map.end(); ++itr) {
                        if (itr == event_map.begin()) continue;
                        if (itr->second.second[0]->pid == c_process.second->pid) {
                            if (itr->second.first == 1) {
                                // print the start cpu burst event
                                cout << "time " << itr->first << "ms: P" << c_process.second->pid
                                     << " started using the CPU ";
            
                                c_process.second->context_switch = false;
                                print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                                int curr_wait_t = itr->first - itr->second.second[0]->time_enterqueue 
                                                  - t_cs;
                                c_process.second->total_wt += curr_wait_t;
                                c_process.second->time_entercpu = it->first;
                 
                                srt_queue.pop();
                                // print the newly arrived one_process arrived event
                                srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                               it->second.second[0]));
                                cout << "time " << itr->first << "ms: P" << it->second.second[0]->pid 
                                     << " arrived ";
                                it->second.second[0]->time_enterqueue = it->first;
                                        
                                print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                                push_event(processes, 
                                           itr->first, 
                                           name_of_algo, 
                                           fcfs_queue, 
                                           srt_queue, 
                                           rr_queue, 
                                           processes_in_tie, 
                                           event_map);
                                // ================================================
                                event_map.erase(itr);
                                // ================================================
                                break;
                                    }
                                }
                            }
                        } else { // if preempt does happens
                            for (customized_map::iterator itr = event_map.begin(); 
                                 itr != event_map.end(); ++itr) {
                                if (itr == event_map.begin()) continue;
                                if (itr->second.second[0]->pid == c_process.second->pid) {
                                    if (itr->second.first == 1) {
                                        cout << "time " << itr->first << "ms: P" 
                                        << itr->second.second[0]->pid << " started using the CPU ";
                                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                                        c_process.second->context_switch = false;
                                        srt_queue.pop();
                                        cout << "time " << itr->first << "ms: P" 
                                             << it->second.second[0]->pid 
                                             << " arrived ,preempting P"
                                             << c_process.second->pid << " ";
                                        
                                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                                        
                                        c_process.second->total_wt += itr->first
                                        - t_cs - c_process.second->time_enterqueue;

                                        it->second.second[0]->preempt_other = true;
                                        srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                                                 it->second.second[0]));
        
                                        it->second.second[0]->time_enterqueue = itr->first;
                                        
                                        push_event(processes, 
                                                   itr->first, 
                                                   name_of_algo, 
                                                   fcfs_queue, 
                                                   srt_queue, 
                                                   rr_queue, 
                                                   processes_in_tie, 
                                                   event_map);
                                        
                                        // readd it to srt_queue
                                        c_process.second->preempting = true;
                                        (c_process.second->extra_num_burst)++;
                                        c_process.second->time_enterqueue = itr->first;
                                        srt_queue.push(make_pair(c_process.second->left_burst_time, 
                                                       c_process.second));
                                    }
                                    event_map.erase(itr);
                                    --itr;
                                    }
                                }
                            }
                        } else { 
                        cout << "time " << it->first << "ms: P" << it->second.second[0]->pid 
                             << " arrived ";
                        srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                       it->second.second[0]));
                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);      
                        it->second.second[0]->time_enterqueue = it->first;
                        push_event(processes, 
                                   it->first, 
                                   name_of_algo, 
                                   fcfs_queue, 
                                   srt_queue, 
                                   rr_queue, 
                                   processes_in_tie, 
                                   event_map);
                    }
                    
                } else { 
                    // If cpu is not empty
                    int curr_remain_t = c_process.second->left_burst_time 
                    - (it->first - c_process.second->time_entercpu);

                    
                    if (curr_remain_t > 0) {
                        if (it->second.second[0]->burst_time < curr_remain_t) 
                            check_preempting = true;
                    }
                    if (check_preempting) {
                        
                        for (customized_map::iterator itr = event_map.begin(); 
                             itr != event_map.end(); ++itr) {
                            if (itr == event_map.begin()) continue;
                            if (itr->second.second[0]->pid == c_process.second->pid) {
                                event_map.erase(itr);
                                --itr;
                            }
                        }
                        
                        cout << "time " << it->first << "ms: P" 
                             << it->second.second[0]->pid << " arrived, preempting P"
                             << c_process.second->pid << " ";
                        
                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);

                        c_process.second->preempting = true;
                        c_process.second->left_burst_time = curr_remain_t;
                        (c_process.second->extra_num_burst)++;
                        c_process.second->time_enterqueue = it->first;
                        
                        srt_queue.push(make_pair(curr_remain_t, c_process.second));
                        srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                                 it->second.second[0]));
                        it->second.second[0]->time_enterqueue = it->first;

                        push_event(processes, 
                                   it->first, 
                                   name_of_algo, 
                                   fcfs_queue, 
                                   srt_queue, 
                                   rr_queue, 
                                   processes_in_tie, 
                                   event_map);
                    } else {
                        srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                                 it->second.second[0]));
                        it->second.second[0]->time_enterqueue = it->first;
                    }
                }
            } else if (name_of_algo == "RR") {
                cout << "time " << it->first << "ms: P" 
                     << it->second.second[0]->pid << " arrived ";
                it->second.second[0]->time_enterqueue = it->first;
                
                // put the newly completed IO one_process into rr_queue
                rr_queue.push(it->second.second[0]); 
                print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                
                // If cpu is empty
                if (c_process.first == 0) 
                    push_event(processes, 
                               it->first, 
                               name_of_algo, 
                               fcfs_queue, 
                               srt_queue, 
                               rr_queue, 
                               processes_in_tie, 
                               event_map);
            } else if (name_of_algo == "FCFS") {
                cout << "time " << it->first << "ms: P" 
                     << it->second.second[0]->pid << " arrived ";
                it->second.second[0]->time_enterqueue = it->first;
                fcfs_queue.push(it->second.second[0]);
                print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);

                if (c_process.first == 0) 
                    push_event(processes,      
                               it->first, 
                               name_of_algo, 
                               fcfs_queue, 
                               srt_queue, 
                               rr_queue, 
                               processes_in_tie, 
                               event_map);
                
            }
        }
    }
    // RR preempting
    if (it->second.first == 4) {
        
        one_process* preempted_process = it->second.second[0];
        
        bool q_empty;
        if (name_of_algo == "RR") q_empty = rr_queue.empty();
        
        if (!q_empty) {
            cout << "time " << it->first << "ms: " 
                 << "Time slice expired, preempting P" << preempted_process->pid << " ";
            
            print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
            
            // remove the complete CPU burst and finish IO events
            for (customized_map::iterator itr = event_map.begin(); 
                 itr != event_map.end(); ++itr) {
                if (itr == event_map.begin()) continue;
                if (itr->second.second[0]->pid == preempted_process->pid) {
                    event_map.erase(itr);
                    --itr;
                }
            }
            
            // push the new events for the next one_process in rr_queue
            push_event(processes, 
                       it->first, 
                       name_of_algo, 
                       fcfs_queue, 
                       srt_queue, 
                       rr_queue, 
                       processes_in_tie, 
                       event_map);
            
            // update time values of preempted_process 
            // and push the preempting one_process back to rr_queue
            int curr_remain_t = preempted_process->left_burst_time - t_slice;
            if (curr_remain_t < 0) curr_remain_t = preempted_process->left_burst_time;
            preempted_process->preempting = true;
            preempted_process->left_burst_time = curr_remain_t;
            ++(preempted_process->extra_num_burst);
            preempted_process->time_enterqueue = it->first;
            rr_queue.push(preempted_process);
        } else {
            if (preempted_process->left_burst_time > t_slice) {
                preempted_process->left_burst_time -= t_slice;
                vector<one_process*> tmp_vec;
                tmp_vec.push_back(preempted_process);
                event_map.insert(make_pair(it->first + t_slice, make_pair(4, tmp_vec)));
            }       
        }
    }


    if (it->second.first == 0) {
        bool check_preempting = false;
        for (unsigned int i = 0; i < it->second.second.size(); ++i) {
            /* add one_process into ready queue */
            if (name_of_algo == "SRT") {
                // if there is no one_process in cpu
                if (c_process.first == 0) {
                    srt_queue.push(make_pair(it->second.second[i]->burst_time, 
                                   it->second.second[i]));
                    cout << "time " << it->first << "ms: P" << it->second.second[i]->pid
                    << " arrived ";
                    print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                    it->second.second[i]->time_enterqueue = it->first;
                }
                // if there is already a one_process running in cpu
                else {
                    // calculate remaining time of c_process in CPU, if it is longer than 
                    // the arriving one_process's burst time, mark the check_preempting as true
                    int curr_remain_t = c_process.second->left_burst_time 
                    + c_process.second->time_entercpu - it->first;
                    if (curr_remain_t > 0) {
                        if (it->second.second[i]->burst_time < curr_remain_t) 
                            check_preempting = true;
                    }

                    // if is preempt
                    if (check_preempting) {
                        cout << "time " << it->first << "ms: P" << it->second.second[i]->pid
                        << " arrived, preempting P" << c_process.second->pid << " ";
                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                        it->second.second[i]->preempt_other = true;
                        
                        // push the new arrived one_process into ready_queue( as the first one)
                        srt_queue.push(make_pair(it->second.second[i]->burst_time, 
                                       it->second.second[i]));
                        
                        /*-----------------------------------------------*/
                        it->second.second[i]->time_enterqueue = it->first;
                        /*-----------------------------------------------*/
                        
                        // and add new events about the new arrived one_process
                        push_event(processes, 
                                   it->first, 
                                   name_of_algo, 
                                   fcfs_queue, 
                                   srt_queue, 
                                   rr_queue, 
                                   processes_in_tie, 
                                   event_map);
                        
                        // remove c_process events from event map
                        for (customized_map::iterator itr = event_map.begin(); 
                             itr != event_map.end(); ++itr) {
                            if (itr == event_map.begin()) continue;
                            if (itr->second.second[0]->pid == c_process.second->pid) {
                                event_map.erase(itr);
                                --itr;
                            }
                        }
                        
                        /* push c_process back into ready q    */
                        c_process.second->preempting = true;
                        c_process.second->left_burst_time = curr_remain_t;
                        ++(c_process.second->extra_num_burst);
                        c_process.second->time_enterqueue = it->first;
                        
                        srt_queue.push(make_pair(curr_remain_t, c_process.second));
                        
                    } else {
                        cout << "time " << it->first << "ms: P" << it->second.second[i]->pid
                        << " arrived ";
                        srt_queue.push(make_pair(it->second.second[0]->burst_time, 
                                                 it->second.second[0]));
                        it->second.second[i]->time_enterqueue = it->first;
                        print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                    }
                    
                }
            }
            else if (name_of_algo == "RR" || name_of_algo == "FCFS") {
                if (name_of_algo == "FCFS") fcfs_queue.push(it->second.second[i]);
                else if (name_of_algo == "RR") rr_queue.push(it->second.second[i]);
                
                cout << "time " << it->first << "ms: P" << it->second.second[i]->pid
                << " arrived ";
                print_given_queue(name_of_algo, fcfs_queue, srt_queue, rr_queue);
                it->second.second[i]->time_enterqueue = it->first;
            }
        }
        /* trigger push event function  */
        if (it->first == 0) 
            push_event(processes, 
                       it->first, 
                       name_of_algo, 
                       fcfs_queue, 
                       srt_queue, 
                       rr_queue, 
                       processes_in_tie, 
                       event_map);
    }
    
    /* catch the final terminated time  */
    if (is_ended(processes)) t = it->first;
    
    /* finally pop event map    */
    event_map.erase(it);
}


bool compare_functor(const one_process& p1, const one_process& p2){
    if (p1.arrival_time != p2.arrival_time) return p1.arrival_time < p2.arrival_time;
    else return p1.pid < p2.pid;
}

/* push RR preemption timeline  */
void slice_adjust_push_event(customized_map& event_map, int& t, one_process* p) {
    vector<one_process*> tmp_v;
    tmp_v.push_back(p);
    event_map.insert(make_pair(t, make_pair(4, tmp_v)));
}


void print_useful_statistcs(ostream& ostr, 
                            const vector<one_process>& processes, 
                            const string& name_of_algo) {
    int CPU_burst_t = 0; 
    int wait_t = 0; 
    int turnaround_t = 0; 
    int num_of_burst = 0; 
    int total_context_switch = 0;
    int num_of_preempt = 0;
    
    /* loop one_process vector  */
    for (unsigned int i = 0; i < processes.size(); ++i) {
        CPU_burst_t += processes[i].burst_time * processes[i].num_of_burst;
        wait_t += processes[i].total_wt;
        turnaround_t += processes[i].burst_time*processes[i].num_of_burst 
                        + processes[i].total_wt;
        num_of_burst += processes[i].num_of_burst;
        total_context_switch += processes[i].num_of_burst 
                              + processes[i].extra_num_burst;
        num_of_preempt += processes[i].extra_num_burst;
        
    }
    
    turnaround_t += total_context_switch * t_cs;
    
    /* caculation of avg time   */
    double avg_CPU_burst = double(CPU_burst_t) / num_of_burst;
    double avg_wait = double(wait_t) / num_of_burst;
    double avg_turnaround = double(turnaround_t) / num_of_burst;
    
    ostr.precision(2);
    ostr << fixed;
    ostr << "Algorithm " << name_of_algo << endl;
    ostr << "-- average CPU burst time: " << avg_CPU_burst << " ms" << endl;
    ostr << "-- average wait time: " << avg_wait << " ms" << endl;
    ostr << "-- average turnaround time: " << avg_turnaround << " ms" << endl;
    ostr << "-- total number of context switches: " << total_context_switch << endl;
    ostr << "-- total number of preemptions: " << num_of_preempt << endl;
}


