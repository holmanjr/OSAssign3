#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>

double timeExecutable(std::string, std::vector<std::string> &, double);
void runExecutable(std::string, std::vector<std::string> &, double);
void ptime(double);
void history(std::vector<std::string> &);

int main(){

	std::string input;//Input from user
	double runTime = 0.0;//Total run time of processes
	std::vector<std::string> hist;//Vector used to hold command history

	//Loop to run the command line
	while(true){
		std::cout << "[cmd]: ";
		std::getline(std::cin, input);
		hist.push_back(input);

		int status;

		if(input == "exit") break;
		else if(input == "ptime"){
			ptime(runTime);
		}
		else if(input == "history"){
			history(hist);
		}
		else{//Not a built in function, adds execution time
			runTime += timeExecutable(input, hist, runTime);
		}
	}

	return 0;
}

//Times the executable only if successful
double timeExecutable(std::string args, std::vector<std::string> &hist, double runTime){
	auto start = std::chrono::system_clock::now();

	int status;
	auto pid = fork();
	if(pid == 0){//Child process
		runExecutable(args, hist, runTime);
	}
	else{//Parent process
		wait(&status);
		if(status != 0){//Execution failed
			hist.pop_back();//Removes failed command from history
			return 0.0;//Does not time failed execution
		}
	}

	auto end = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

	return diff.count()/1000000.0;
}

//Creates correct data types and attempts to run executable
void runExecutable(std::string args, std::vector<std::string> &hist, double runTime){
	std::stringstream ss;
	std::string component;
	ss.str(args);
	int n = 0;
	std::vector<std::string> components;

	//Create vector with individual arguments
	while(std::getline(ss,component,' ')){
		n++;
		components.push_back(component);
	}

	//Execute command from history
	if(components[0] == "^"){
		if((hist[std::stoi(components[1])-1])== "ptime"){
			ptime(runTime);
			exit(0);
		}
		else if((hist[std::stoi(components[1])-1])== "history"){
			history(hist);
			exit(0);
		}
		else runExecutable(hist[std::stoi(components[1])-1], hist, runTime);
		return;
	}

	char** argv;
	argv = new char*[n];

	//Unload arguments from vector into correct data types
	for(int i = 0; i<n; i++){
		argv[i] = new char[components[i].size()+1];
		strcpy(argv[i], components[i].c_str());
	}
	argv[n] = NULL;

	//Run execution inside if statement
	if(execvp(argv[0], argv)<0){//Execution failed
		std::cout << "ERROR: exec failed" << std::endl;
		exit(1);
	}
}

void ptime(double runTime){
	std::cout << "Time spent executing child processes: ";
	std::cout << std::fixed << std::setprecision(4) << runTime;
	std::cout << " seconds" << std::endl;
}

void history(std::vector<std::string> &hist){
	std::cout << "-- Command History --\n\n";
	for(int i = 0; i < hist.size(); i++){
		std::cout << i+1 << " : " << hist[i] << std::endl;
	}
	std::cout << std::endl;
}
