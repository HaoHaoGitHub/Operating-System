// Class for each process
#include <iostream>


class one_process {
public:
	one_process() {
		proc_num = 0;
		burst_time = 0;
		num_brust = 0;
		io_time = 0;
		wait_time = 0;

	};
	one_process(int p, int b, int n, int i, int w) {
		proc_num = p;
		burst_time = b;
		num_brust = n;
		io_time = i;
		wait_time = w;
	};

	// Accessors:
	int get_proc_num() const {return proc_num;}
	int get_burst_time() const {return burst_time;}
	int get_num_burst() const {return num_brust;}
	int get_io_time() const {return io_time;}
	int get_wait_time() const {return wait_time;}

	// Modifiers:
	void set_p(int p);
	void set_b(int b);
	void set_n(int n);
	void set_i(int i);
	void set_w(int w);

	
private:
	int proc_num;
	int burst_time;
	int num_brust;
	int io_time;
	int wait_time;
};



bool less_num(const one_process& p1, const one_process& p2) {
	return p1.get_proc_num() < p2.get_proc_num();
}

bool less_burst(const one_process& p1, const one_process& p2) {
	return p1.get_burst_time() < p2.get_burst_time();

}

void one_process::set_p(int p) {
	proc_num = p;
}

void one_process::set_b(int b) {
	burst_time = b;
}

void one_process::set_n(int n) {
    num_brust = n;
}

void one_process::set_i(int i) {
	io_time = i;
}

void one_process::set_w(int w) {
	wait_time = w;
}






