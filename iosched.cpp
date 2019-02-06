#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <list>
#include <algorithm>
#include <iomanip>
#include <unistd.h>
using namespace std;

class request {
public:
	int id;
	int arrive_time;
	int start_time;
	int complete_time;
	int track;
};

list<int> ioqueue;
vector<request*> reqs;
list<int> requeue[2];
list<int> *active = &requeue[0];
list<int> *add = &requeue[1];
int head = 0;
int direction = 0;
int current = -1;

class scheduler {
public:
	virtual int select_request();
	virtual void add_request(int id);
};

int scheduler::select_request() {
	return 0;
}

void scheduler::add_request(int id){}

class FIFO :public scheduler {
public:
	int select_request();
	void add_request(int id);
};

int FIFO::select_request() {
	if (!ioqueue.empty()) {
		int id = ioqueue.front();
		ioqueue.pop_front();
		return id;
	}
	else
		return -1;
}

void FIFO::add_request(int id) {
	ioqueue.push_back(id);
}

class SSTF:public scheduler {
public:
	int select_request();
	void add_request(int id);
};

int SSTF::select_request() {
	if (ioqueue.empty())
		return -1;
	int id = ioqueue.front();
	list<int>::iterator iter;
	iter = ioqueue.begin();
	iter++;
	while (iter != ioqueue.end()) {
		if (abs(head - reqs[(*iter)]->track) < abs(head - reqs[id]->track)) {
			id = (*iter);
		}
		iter++;
	}
	for (iter = ioqueue.begin(); iter != ioqueue.end(); iter++) {
		if ((*iter) == id) {
			ioqueue.erase(iter);
			break;
		}
	}
	return id;
}

void SSTF::add_request(int id) {
	ioqueue.push_back(id);
}

class LOOK :public scheduler {
public:
	int select_request();
	void add_request(int id);
};

int LOOK::select_request() {
	if (ioqueue.empty())
		return -1;
	int id = -1;
	list<int>::iterator iter = ioqueue.begin();
	if (direction == 0) {
		while (iter != ioqueue.end()) {
			if (reqs[(*iter)]->track >= head){
				id = (*iter);
				iter++;
				while (iter != ioqueue.end()) {
					if (reqs[(*iter)]->track >= head && abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
						id = (*iter);
					iter++;
				}
				break;
			}
			else {
				iter++;
				continue;
			}
		}
	}
	else if (direction == 1) {
		while (iter != ioqueue.end()) {
			if (reqs[(*iter)]->track <= head) {
				id = (*iter);
				iter++;
				while (iter != ioqueue.end()) {
					if (reqs[(*iter)]->track <= head && abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
						id = (*iter);
					iter++;
				}
				break;
			}
			else {
				iter++;
				continue;
			}
		}
	}
	if (id == -1) {
		id = ioqueue.front();
		iter = ioqueue.begin();
		iter++;
		while (iter != ioqueue.end()) {
			if (abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
				id = (*iter);
			iter++;
		}
	}
	for (iter = ioqueue.begin(); iter != ioqueue.end(); iter++) {
		if ((*iter) == id) {
			ioqueue.erase(iter);
			break;
		}
	}
	return id;
}

void LOOK::add_request(int id) {
	ioqueue.push_back(id);
}

class CLOOK :public scheduler {
public:
	int select_request();
	void add_request(int id);
};

int CLOOK::select_request() {
	if (ioqueue.empty())
		return -1;
	int id = -1;
	list<int>::iterator iter = ioqueue.begin();
	while (iter != ioqueue.end()) {
		if (reqs[(*iter)]->track >= head) {
			id = (*iter);
			iter++;
			while (iter != ioqueue.end()) {
				if (reqs[(*iter)]->track >= head && abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
					id = (*iter);
				iter++;
			}
			break;
		}
		else {
			iter++;
			continue;
		}
	}
	if (id == -1) {
		id = ioqueue.front();
		iter = ioqueue.begin();
		iter++;
		while (iter != ioqueue.end()) {
			if (reqs[(*iter)]->track < reqs[id]->track)
				id = (*iter);
			iter++;
		}
	}
	for (iter = ioqueue.begin(); iter != ioqueue.end(); iter++) {
		if ((*iter) == id) {
			ioqueue.erase(iter);
			break;
		}
	}
	return id;
}

void CLOOK::add_request(int id) {
	ioqueue.push_back(id);
}

class FSCAN :public scheduler {
public:
	int select_request();
	void add_request(int id);
};

int FSCAN::select_request() {
	if (active->empty()) {
		list<int> *temp = active;
		active = add;
		add = temp;
	}
	if (active->empty())
		return -1;
	int id = -1;
	list<int>::iterator iter = active->begin();
	if (direction == 0) {
		while (iter != active->end()) {
			if (reqs[(*iter)]->track >= head) {
				id = (*iter);
				iter++;
				while (iter != active->end()) {
					if (reqs[(*iter)]->track >= head && reqs[(*iter)]->track - head < abs(reqs[id]->track - head)) 
						id = (*iter);
					iter++;
				}
				break;
			}
			else {
				iter++;
				continue;
			}
		}
		if (id == -1) {
			direction = 1;
			iter = active->begin();
			id = active->front();
			while (iter != active->end()) {
				if (abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
					id = *iter;
				iter++;
			}
		}
	}
	else {
		while (iter != active->end()) {
			if (reqs[(*iter)]->track <= head) {
				id = (*iter);
				iter++;
				while (iter != active->end()) {
					if (reqs[(*iter)]->track <= head && abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head)) {
						id = (*iter);
					}
					iter++;
				}
				break;
			}
			else {
				iter++;
				continue;
			}
		}
		if (id == -1) {
			direction = 0;
			iter = active->begin();
			id = active->front();
			while (iter != active->end()) {
				if (abs(reqs[(*iter)]->track - head) < abs(reqs[id]->track - head))
					id = *iter;
				iter++;
			}
		}
	}
	for (iter = active->begin(); iter != active->end(); iter++) {
		if ((*iter) == id) {
			active->erase(iter);
			break;
		}
	}
	return id;
}

void FSCAN::add_request(int id) {
	add->push_back(id);
}

static const char *optString = "s:";

int main(int argc, char* argv[])
{
	char* inputname = NULL;
	char* schedname = NULL;
	/* read the options */
	int opt = getopt(argc, argv, optString);
	while (opt != -1) {
		switch (opt) {
		case 's':
			schedname = optarg;
			break;

		default:
			abort();
			break;
		}
		opt=getopt(argc, argv, optString);
	}
	int index = optind;
	inputname = argv[index];
	string s = schedname;
	scheduler* sched;
	if (s == "i")
		sched = new FIFO();
	else if (s == "j")
		sched = new SSTF();
	else if (s == "s")
		sched = new LOOK();
	else if (s == "c")
		sched = new CLOOK();
	else if (s == "f")
		sched = new FSCAN();
 
	/* read the input file */
	ifstream input(inputname);
	char buffer[50];
	while (!input.eof()) {
		input.getline(buffer, 50);
		if (buffer[0] == '#' || buffer[0] == '\0')
			continue;
		request* newreq = new request();
		newreq->arrive_time = atoi(strtok(buffer, " "));
		newreq->track = atoi(strtok(NULL, " "));
		reqs.push_back(newreq);
	}
	int time = 0;
	int id = 0;
	int total_waittime = 0;
	int total_turnaround = 0;
	int max_waittime = 0;
	int total_movement = 0;
	int complete_time = 0;
	int start_time = 0;
	/* simulation */
	while (true) {
		if (id < reqs.size() && reqs[id]->arrive_time == time) {
			/* a new operation arrives */
			sched->add_request(id);
			if (id < reqs.size())
				id++;
		}
		if (current != -1){
			if (reqs[current]->track == head) {
				reqs[current]->complete_time = time;
				complete_time = time;
				total_turnaround += reqs[current]->complete_time - reqs[current]->arrive_time;
				current = -1;
				if (ioqueue.empty() && active->empty() && add->empty() && id == reqs.size())
					break;
			}
			else if (reqs[current]->track > head) {
				head++;
				time++;
				direction = 0;
			}
			else if (reqs[current]->track < head) {
				head--;
				time++;
				direction = 1;
			}
		}
		if (current == -1) {
			current = sched->select_request();
			if (current == -1) {
				time++;
				continue;
			}
			reqs[current]->start_time = time;
			total_waittime += time - reqs[current]->arrive_time;
			max_waittime = max(max_waittime, time - reqs[current]->arrive_time);
			total_movement += abs(head - reqs[current]->track);
		}
		
	}
	int i = 0;
	for (i = 0; i < reqs.size(); i++) {
		cout << setw(5) << i << ": " << setw(5) << reqs[i]->arrive_time << " "
			<< setw(5) << reqs[i]->start_time << " " << setw(5) << reqs[i]->complete_time << endl;
	}
	cout << "SUM: " << complete_time << " " << total_movement << " " << fixed << setprecision(2) << (double)total_turnaround / id
		<< " " << fixed << setprecision(2) << (double)total_waittime / id << " " << max_waittime << endl;
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
