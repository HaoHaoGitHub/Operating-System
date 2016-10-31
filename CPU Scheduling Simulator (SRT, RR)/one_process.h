/* one_process Class. */
#include <iostream>

using namespace std;

class one_process {
public:
    /* Constructor function */
    one_process() {
        pid = -1;

    }

    one_process(int p, int at, int bt, int nt, int It ) {
    	pid = p;
    	arrival_time = at;
    	burst_time = bt;
    	left_burst_time = bt;
    	num_of_burst = nt;
    	left_num_burst = nt;
    	extra_num_burst = 0;
    	io_time = It;
    	total_wt = 0;
    	preempting = false;
    	preempt_other = false; 
    	context_switch = false; /* Which means in context switch */
        time_entercpu = 0;
        time_enterqueue = 0;
    }

    void set_original() {
        left_num_burst = num_of_burst;
        time_enterqueue = 0;
        time_entercpu = 0;
        total_wt = 0;
        left_burst_time = burst_time;
        preempting = false;
        preempt_other = false;
        context_switch = false;
        extra_num_burst = 0;
    }


    int pid;
    int arrival_time;
    int burst_time;
    int left_burst_time;
    int num_of_burst;
    int left_num_burst;
    int extra_num_burst;
    int io_time;
    int total_wt;
    bool preempting;
    bool preempt_other;
    bool context_switch;
    int time_enterqueue;
    int time_entercpu;
};




