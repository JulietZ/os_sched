#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <climits>
#include <string>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>
#include <ctype.h>
#include <unistd.h>
#include <limits>
using namespace std;

int FIFO=1;
int SSTF=2;
int SCAN=3;
int CSCAN=4;
int FSCAN=5;
int TYPE=5;
int flag=1;
int listflag=1;

int timestamp=0;
double tempNo;
int track=0;
int ioNo=0;
int tot_movement=0;
double avg_turnaround;
double avg_waittime;
int max_waittime=0;
int tot_waittime=0;
int tot_turnaround=0;

struct event{
    int time;
    int trackNo;
    int seqNo;
    int begin_time;
    int finish_time;
};
list<event> ioWaitlist;
list<event> ioWaitlist2;
vector<event> allevent;

int main(int argc, char* argv[]) {
    char* algo=NULL;
    int c;
    
    while ((c = getopt (argc, argv, "s:")) != -1)
    switch (c){
            case 's':
            algo = optarg;//i,j,s,c,f
            break;
        default:
            abort ();
    }
    
    if (strncmp(algo,"i",1)==0) TYPE=FIFO;
     else if (strncmp(algo,"j",1)==0) TYPE=SSTF;
     else if (strncmp(algo,"s",1)==0) TYPE=SCAN;
     else if (strncmp(algo,"c",1)==0) TYPE=CSCAN;
     else if (strncmp(algo,"f",1)==0) TYPE=FSCAN;
    
    ifstream infile(argv[optind]);
    if (!infile) {
        cout << "Fail to open infile" << endl;
    }
    
    int time;
    int trackNo;
    string line;
    
    while (!infile.eof()) {
        getline(infile,line);
        if (line[0]=='#' || line.length()==0) {
            continue;
        }
        
        istringstream iss(line);
        iss>>time>>trackNo;
        event e;
        e.time=time;
        e.trackNo=trackNo;
        e.seqNo=ioNo++;
        e.begin_time=0;
        e.finish_time=0;
        
        allevent.push_back(e);
    }

    int index = 0;
    if (TYPE==FSCAN) {
        while (index < allevent.size()||!ioWaitlist.empty()||!ioWaitlist2.empty()){
            if (ioWaitlist.empty()&&ioWaitlist2.empty()) {
                timestamp=allevent[index].time;
                ioWaitlist.push_back(allevent[index]);
                index++;
                listflag=1;
            }
            else {
                event issue;
                int temp=std::numeric_limits<int>::max();
                list<event>::iterator tempIssue;
                if (listflag==1&&ioWaitlist.empty()) {
                    listflag=2;
                    flag=1;
                }
                else if(listflag==2&&ioWaitlist2.empty()){
                    listflag=1;
                    flag=1;
                }
                if (listflag==1){
                    if (flag==1) {
                        for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                            if ((*it).trackNo>=track) {
                                int range=(*it).trackNo-track;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=0;
                        }
                    }
                    if (flag==0) {
                        for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                            if ((*it).trackNo<=track) {
                                int range=track-(*it).trackNo;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=1;
                            for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                                if ((*it).trackNo>track) {
                                    int range=(*it).trackNo-track;
                                    if (range<temp) {
                                        temp=range;
                                        tempIssue=it;
                                    }
                                }
                            }
                        }
                    }
                    issue=*tempIssue;
                    ioWaitlist.erase(tempIssue);
                }
                else if (listflag==2){
                    if (flag==1) {
                        for (std::list<event>::iterator it=ioWaitlist2.begin(); it != ioWaitlist2.end(); ++it){
                            if ((*it).trackNo>=track) {
                                int range=(*it).trackNo-track;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=0;
                            for (std::list<event>::iterator it=ioWaitlist2.begin(); it != ioWaitlist2.end(); ++it){
                                if ((*it).trackNo<=track) {
                                    int range=track-(*it).trackNo;
                                    if (range<temp) {
                                        temp=range;
                                        tempIssue=it;
                                    }
                                }
                            }
                        }
                    }
                    else if (flag==0) {
                        for (std::list<event>::iterator it=ioWaitlist2.begin(); it != ioWaitlist2.end(); ++it){
                            if ((*it).trackNo<=track) {
                                int range=track-(*it).trackNo;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=1;
                            for (std::list<event>::iterator it=ioWaitlist2.begin(); it != ioWaitlist2.end(); ++it){
                                if ((*it).trackNo>=track) {
                                    int range=(*it).trackNo-track;
                                    if (range<temp) {
                                        temp=range;
                                        tempIssue=it;
                                    }
                                }
                            }
                        }
                    }
                    issue=*tempIssue;
                    ioWaitlist2.erase(tempIssue);
                }
                allevent[issue.seqNo].begin_time=timestamp;
                tot_movement+=abs(track-issue.trackNo);
                timestamp+=abs(track-issue.trackNo);
                track=issue.trackNo;
                while (index<allevent.size()&&allevent[index].time<=timestamp) {
                    if (listflag==1) {
                        ioWaitlist2.push_back(allevent[index]);
                    }
                    else {
                        ioWaitlist.push_back(allevent[index]);
                    }
                    index++;
                }
                allevent[issue.seqNo].finish_time=timestamp;
                int tempWaitTime=allevent[issue.seqNo].begin_time-allevent[issue.seqNo].time;
                int tempTurnaround=timestamp-allevent[issue.seqNo].time;
                tot_waittime+=tempWaitTime;
                tot_turnaround+=tempTurnaround;
                if (tempWaitTime>max_waittime) {
                    max_waittime=tempWaitTime;
                }
            }
        }
    }
    else{
        while (index < allevent.size()||!ioWaitlist.empty()){
            if (ioWaitlist.empty()) {
                timestamp=allevent[index].time;
                ioWaitlist.push_back(allevent[index]);
                index++;
            }
            if (!ioWaitlist.empty()) {
                event issue;
                if (TYPE==FIFO) {
                    issue=ioWaitlist.front();
                    ioWaitlist.pop_front();
                }
                else if (TYPE==SSTF) {
                    int temp=std::numeric_limits<int>::max();
                    list<event>::iterator tempIssue;
                    for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                        int range=abs((*it).trackNo-track);
                        if (range<temp) {
                            temp=range;
                            tempIssue=it;
                        }
                    }
                    issue=*tempIssue;
                    ioWaitlist.erase(tempIssue);
                }
                else if (TYPE==SCAN) {
                    int temp=std::numeric_limits<int>::max();
                    list<event>::iterator tempIssue;
                    if (flag==1) {
                        for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                            if ((*it).trackNo>=track) {
                                int range=(*it).trackNo-track;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=0;
                            for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                                if ((*it).trackNo<track) {
                                    int range=track-(*it).trackNo;
                                    if (range<temp) {
                                        temp=range;
                                        tempIssue=it;
                                    }
                                }
                            }
                        }
                    }
                    if (flag==0) {
                        for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                            if ((*it).trackNo<=track) {
                                int range=track-(*it).trackNo;
                                if (range<temp) {
                                    temp=range;
                                    tempIssue=it;
                                }
                            }
                        }
                        if (temp==std::numeric_limits<int>::max()) {
                            flag=1;
                            for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                                if ((*it).trackNo>track) {
                                    int range=(*it).trackNo-track;
                                    if (range<temp) {
                                        temp=range;
                                        tempIssue=it;
                                    }
                                }
                            }
                        }
                    }
                    issue=*tempIssue;
                    ioWaitlist.erase(tempIssue);
                }
                else if (TYPE==CSCAN) {
                    int temp=std::numeric_limits<int>::max();
                    list<event>::iterator tempIssue;
                    for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                        if ((*it).trackNo>=track) {
                            int range=(*it).trackNo-track;
                            if (range<temp) {
                                temp=range;
                                tempIssue=it;
                            }
                        }
                    }
                    if (temp==std::numeric_limits<int>::max()) {
                        for (std::list<event>::iterator it=ioWaitlist.begin(); it != ioWaitlist.end(); ++it){
                            if ((*it).trackNo<temp) {
                                temp=(*it).trackNo;
                                tempIssue=it;
                            }
                        }
                    }
                    issue=*tempIssue;
                    ioWaitlist.erase(tempIssue);
                }
                allevent[issue.seqNo].begin_time=timestamp;
                tot_movement+=abs(track-issue.trackNo);
                timestamp+=abs(track-issue.trackNo);
                track=issue.trackNo;
                while (index<allevent.size()&&allevent[index].time<=timestamp) {
                    ioWaitlist.push_back(allevent[index]);
                    index++;
                }
                allevent[issue.seqNo].finish_time=timestamp;
                int tempWaitTime=allevent[issue.seqNo].begin_time-allevent[issue.seqNo].time;
                int tempTurnaround=timestamp-allevent[issue.seqNo].time;
                tot_waittime+=tempWaitTime;
                tot_turnaround+=tempTurnaround;
                if (tempWaitTime>max_waittime) {
                    max_waittime=tempWaitTime;
                }
            }
        }
    }
    tempNo=(double)allevent.size();
    avg_waittime=tot_waittime/tempNo;
    avg_turnaround=tot_turnaround/tempNo;
    printf("SUM: %d %d %.2lf %.2lf %d\n", timestamp,
           tot_movement, avg_turnaround, avg_waittime, max_waittime);
}





















