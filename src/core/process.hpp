#pragma once

#ifndef PROCESS_HPP_NEROSHOP
#define PROCESS_HPP_NEROSHOP

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace neroshop {
class Process {
public:
    Process();
	Process(const std::string& program, const std::string& argument);
	~Process();
	
    void * open();
	bool create(const std::string& program, const std::string& argument);
	bool terminate();
	static bool terminate(const Process& process);//void kill(); // linux
	static void terminate_by_process_id(int process_id);
	static void terminate_by_process_name(const std::string& process_name);
	static void show_processes(); // pname, pid, pstatus
	void exit(int code = 0);
	// getters
#ifdef _WIN32
	void * get_handle() const;
#endif
#ifdef __gnu_linux__
    int get_handle() const;
    std::string get_name() const;
    static int get_process_by_name(const std::string& process_name);
#endif
	static void * get_active();
///////////////////////////////
private:
#ifdef _WIN32
    void * handle;
#endif  
#ifdef __gnu_linux__
    int handle;
    std::string name;
#endif  
    static std::vector<std::tuple<std::string, int, bool>> process_list; // holds process_name (string) and process_id (int), process_status (bool)
}; // Process process("glue.exe", " srlua.exe test.lua test.exe");
}
#endif
