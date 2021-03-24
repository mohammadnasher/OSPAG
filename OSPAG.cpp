//
//  main.cpp
//  _OS_Final
//
//  Created by Thenonchetable on 12/5/19.
//  Copyright © 2019 Mohammad. All rights reserved.
//
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>



auto constexpr __MEMORY_SIZE__ = 1000;
auto constexpr __FRAME_SIZE__ = 200;
//using mutex to avoid race condition
std::mutex mtx;


void _LogError(std::string e){
    std::cout<< e <<std::endl;
    system("pause");
    return;
}

void _LogOutput(std::string log){
    std::cout<< log <<std::endl;
    return;
}

struct Process{
    //the number which refered to process
    std::string _ProcNum;
    //start and finish time
    int _StartTime;
    int _Deadline;
    //set of needed memory
    std::vector<int> _NeedMemory;
};

struct Frame{
    //the name of process which is in this frame
    std::string _ProcName;
    //Frame Number
    size_t _FrameNumber;
    //Frame Deadline
    int _Deadline;
};


class OsSimulator{
private:
    //Process list. it's like a secondary memory.
    std::vector<Process> _ProcList;
    //main memory (or typically the RAM)
    std::vector<Frame> _MainMemory;
    //all the processes first will come to queue
    std::vector<Process> _Queue;
    //system clock for synchronizing
    int _clockCycle;
    //number of total processes
    int _NumOfProcess;
    //number of empty frames in main memory
    int _NumofEmptyFrames;
    //Convert a process to frames and put them into secondary memory
    std::vector<Frame> _proc2frame(std::string name, int psize, size_t pNum, int dead){
        std::vector<Frame> res;
        if(psize % __FRAME_SIZE__ > 0){
            for(auto i = 0 ; i < (psize / __FRAME_SIZE__) + 1 ; i++){
                Frame tmp;
                tmp._ProcName = name;
                tmp._FrameNumber = pNum;
                tmp._Deadline = dead;
                res.push_back(tmp);
            }
        }
        else{
            for(auto i = 0 ; i < psize / __FRAME_SIZE__ ; i++){
                Frame tmp;
                tmp._ProcName = name;
                tmp._FrameNumber = pNum;
                tmp._Deadline = dead;
                res.push_back(tmp);
            }
        }
        return res;
    }
    //initialize the main memory. create frames and fill memory with them.
    void _memoryInitializer(){
        this->_NumofEmptyFrames = __MEMORY_SIZE__/ __FRAME_SIZE__;
        for(int i = 0 ; i < __MEMORY_SIZE__/ __FRAME_SIZE__ ; i++){
            Frame tmp;
            this->_MainMemory.push_back(tmp);
        }
    }
    //delete a process from main
    void _deleteFromMemory(std::string inp){
        for(auto i = 0 ; i < this->_MainMemory.size(); i++){
            if(this->_MainMemory[i]._ProcName == inp){
                this->_MainMemory[i]._ProcName = "";
                this->_NumofEmptyFrames++;
            }
        }
    }
    void _memoryAllocator(std::vector<Frame> inp){
        auto tmp = inp;
        for(auto i = 0 ; i < this->_MainMemory.size(); i++){
            if(tmp.size() == 0)
                return;
            //if the frame is empty
            if(this->_MainMemory[i]._ProcName == ""){
                this->_MainMemory[i] = tmp.back();
                tmp.pop_back();
                this->_NumofEmptyFrames--;
            }
        }
    }
    
    bool is_memory_empty(){
        for(auto i = 0 ; i < this->_MainMemory.size(); i++){
            if(this->_MainMemory[i]._ProcName != "")
                return false;
        }
        return true;
    }
    
    //print function
    void print(){
        std::cout << "main memory content: \n";
        for(auto i = 0 ; i < this->_MainMemory.size(); i++){
            std::cout<<this->_MainMemory[i]._ProcName<<":"<<this->_MainMemory[i]._Deadline<<"\t";
        }
        std::cout<<std::endl;
        if(this->_Queue.size() == 0)
            std::cout<< "queue is empty!"<<std::endl;
        else
        {
            std::cout << "queue content: \n";
            for(auto i = 0 ; i < this->_Queue.size(); i++){
                std::cout<< this->_Queue[i]._ProcNum << "\t";
            }
            std::cout << std::endl;
        }
        std::cout<<"---------------------------------------------------------------"<<std::endl;
    }
    
    void _reInitializer(std::string File_Address){
        mtx.lock();
        //if the argument is an empty string
        if(File_Address == ""){
            _LogError("Invalid file address!");
            mtx.unlock();
            return;
        }
        std::fstream File;
        File.open(File_Address);
        //file opened successfuly
        if(File.is_open()){
            this->print();
            std::cout<<"start to re intitialize"<<std::endl;
            int num;
            File >> num;
            int cnt;
            //read process information from the file and do this clishes same for each process
            for(int i = 0 ; i < num; i++){
                Process tmp;
                File >> tmp._ProcNum;
                File >> tmp._StartTime;
                File >> tmp._Deadline;
                File >> cnt;
                int memorycounter;
                for(int j = 0 ; j < cnt; j++){
                    File >> memorycounter;
                    tmp._NeedMemory.push_back(memorycounter);
                }
                this->_Queue.push_back(tmp);
            }
        }
        //there is a problem with opening file
        else{
            _LogError("the input file is damaged or the input file address in incorrect!");
            mtx.unlock();
            return;
        }
        this->print();
        mtx.unlock();
    }
    
public:
    //OsSimulator Constructor -> to initialize simulator properties
    OsSimulator(std::string File_Address){
        //if the argument is an empty string
        if(File_Address == ""){
            _LogError("Invalid file address!");
            return;
        }
        std::fstream File;
        File.open(File_Address);
        //file opened successfuly
        if(File.is_open()){
            File >> this->_NumOfProcess;
            int cnt;
            //read process information from the file and do this clishes same for each process
            for(int i = 0 ; i < this->_NumOfProcess; i++){
                Process tmp;
                File >> tmp._ProcNum;
                File >> tmp._StartTime;
                File >> tmp._Deadline;
                File >> cnt;
                int memorycounter;
                for(int j = 0 ; j < cnt; j++){
                    File >> memorycounter;
                    tmp._NeedMemory.push_back(memorycounter);
                }
                this->_Queue.push_back(tmp);
            }
            //initialize memory with given conditions (frame size and memory size)
            this->_memoryInitializer();
        }
        //there is a problem with opening file
        else{
            _LogError("the input file is damaged or the input file address in incorrect!");
            return;
        }
    }
    
    
    void print_processes(){
        for(int i = 0 ; i < this->_Queue.size(); i++){
            std::cout<<this->_Queue[i]._ProcNum<<std::endl;
            std::cout<<this->_Queue[i]._StartTime<<"\t"<<this->_Queue[i]._Deadline<<std::endl;
            std::cout<<this->_Queue[i]._NeedMemory.size() <<"\t";
            for(int j = 0 ; j < this->_Queue[i]._NeedMemory.size(); j++){
                std::cout<< this->_Queue[i]._NeedMemory[j]<<"\t";
            }
            std::cout << std::endl;
        }
    }
    
    
    void fileListener(){
        std::string tmp;
        std::cout<<"Please enter name of new text file which contains the processes detail: "<<std::endl;
        std::cin>>tmp;
        this->_reInitializer(tmp);
    }
    
    
    
    void startSimulation(){
        //always the start clock in a computer will be set to zero
        this->_clockCycle = 0;
        std::vector<std::string> completed;
        //Do while the queue is not empty
        while(!this->_Queue.empty() || !this->is_memory_empty()){
            mtx.lock();
            _LogOutput("T = " + std::to_string(this->_clockCycle));
            //this->print();
            //checking Main Memory
            for(auto i = 0 ; i < this->_MainMemory.size(); i++){
               //if the deadline of a frame expired delete it
               if(this->_MainMemory[i]._ProcName != "" && this->_MainMemory[i]._Deadline <= this->_clockCycle){
                   //the completed message has not been loged
                   _LogOutput("Process which named "+ this->_MainMemory[i]._ProcName + "completed");
                   this->_deleteFromMemory(this->_MainMemory[i]._ProcName);
               }
            }
            //Allocation process into the memory
            //for all processes in the queue
            for(int i = 0 ; i< this->_Queue.size(); i++){
                if(this->_Queue[i]._StartTime <= this->_clockCycle && this->_Queue[i]._Deadline > this->_clockCycle){
                for(size_t j = 0; j < this->_Queue[i]._NeedMemory.size(); j++){
                    if(this->_Queue[i]._NeedMemory[j] <= this->_NumofEmptyFrames * __FRAME_SIZE__){
                        //convert the process to frames and put it into the main memory
                        _LogOutput("Process " + this->_Queue[i]._ProcNum + " page number "+ std::to_string(j+1) + "moved to memeory");
                        auto tmp = this->_proc2frame(this->_Queue[i]._ProcNum, this->_Queue[i]._NeedMemory[j], j, this->_Queue[i]._Deadline);
                        this->_Queue[i]._NeedMemory.erase(this->_Queue[i]._NeedMemory.begin() + j);
                        this->_memoryAllocator(tmp);
                        }
                    }
                    //if the process completelly allocated -> delete from the queue and log permentally moved to main memory
                    if(this->_Queue[i]._NeedMemory.size() == 0){
                        _LogOutput("Process " + this->_Queue[i]._ProcNum + " completely allocated :)");
                        this->_Queue.erase(this->_Queue.begin() + i);
                    }
                }
                //if process's deadline has been expired -> delete it from queue + delete from main memory + log failure
                else if(this->_Queue[i]._Deadline < this->_clockCycle){
                    _LogOutput("process which named" + this->_Queue[i]._ProcNum + "has been failed because the deadline has been expired!");
                    this->_deleteFromMemory(this->_Queue[i]._ProcNum);
                    this->_Queue.erase(this->_Queue.begin()+i-1);
                }
            }
            //the t (time) will be an incremental sycnchronous
            this->_clockCycle++;
            mtx.unlock();
        }
    }
};


int main(){
    
    OsSimulator* omidOS = new OsSimulator("OS.txt");
    //Thread 1 -> start initializing and paging
    std::thread th1(&OsSimulator::startSimulation, omidOS);
    
    //Thread 2-> waiting for input new file address
    std::thread th2(&OsSimulator::fileListener, omidOS);
    
    th1.join();
    th2.join();
    
    return 0;
}
