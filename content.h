// Class for each process in the event priority queue
#include <iostream>
#include <assert.h>


class content {
public:
	content() {
		pid = 0;
		burst_time = 0;
		context_switch = 0;
	}
	content(int p, int b, int cs) {
		pid = p;
		burst_time = b;
		context_switch = cs;
	}
    
    // Accessors:
    int get_pid() const {return pid;}
    int get_burst() const {return burst_time;}
    int get_context_switch() const {return context_switch;}
    // Modifier:
    void set_cs(int cs);

	int pid;
	int burst_time;
	int context_switch;
}; 

bool nearer (const content& c1, const content& c2) {
     if (c1.get_context_switch() > c2.get_context_switch()) {
     	return true;
     } else if (c1.get_context_switch() < c2.get_context_switch()) {
     	return false;
     } else {
     	assert(c1.get_context_switch() == c2.get_context_switch());
     	if (c1.get_burst() < c2.get_burst()) return true;
        else if (c1.get_burst() == c2.get_burst() && c1.get_pid() < c2.get_pid()) return true; 
     	else return false;
     }
}

void content::set_cs(int cs) {
	context_switch = cs;
}



