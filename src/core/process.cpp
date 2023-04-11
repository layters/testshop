#include "process.hpp"

#include "util/logger.hpp"
#include "util.hpp"

neroshop::Process::Process()
{
#ifdef __gnu_linux__
    handle = -1; // default
#endif
}
////////////////////
neroshop::Process::Process(const std::string& program, const std::string& arg) : Process()
{
    if(!create(program, arg)) {
		neroshop::print("Process creation failed", 1);
	}
}
////////////////////
neroshop::Process::~Process()
{
#ifdef __gnu_linux__
#ifdef DOKUN_DEBUG0
    std::cout << DOKUN_UI_TAG "process (" << name << ") has been deallocated" << std::endl;
#endif	
#endif
	terminate(); // kill pid
}
////////////////////
std::vector<std::tuple<std::string, int, bool>> neroshop::Process::process_list({});
////////////////////	
void * neroshop::Process::open()
{
#ifdef _WIN32
	this->handle = static_cast<void *>(OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId()));
	return this->handle;
#endif	
#ifdef __gnu_linux__
#endif
    return nullptr;
}
////////////////////
bool neroshop::Process::create(const std::string& program, const std::string& argument)
{
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory (&si, sizeof(si));
    si.cb = sizeof (si);
    ZeroMemory (&pi, sizeof(pi));
       // static_cast<LPSTR>(const_cast<char *>(program.c_str()))
	std::string cmd = program + argument;   
    // Start the child process. 
    if(CreateProcess( 
	    NULL,//static_cast<LPCTSTR>("glue.exe"),   // No module name (use command line)
        static_cast<LPSTR>(const_cast<char *>(cmd.c_str())),//static_cast<LPCTSTR>("glue.exe srlua.exe main.lua main.exe"),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    == 0) 
    {
        return false;
    }
    // Wait until child process exits.
    WaitForSingleObject (pi.hProcess, INFINITE);
    // Close process and thread handles. 
    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread );
#endif	
#ifdef __gnu_linux__
    pid_t child_pid = fork (); // holds the process id (pid)
    if (child_pid != 0) {
        // the handle is basically the process id (pid)
		this->handle = static_cast<pid_t>(child_pid);
		this->name = program.substr(program.find_last_of("\\/") + 1);//program;
		// store process information in "process_list"
		process_list.push_back(std::make_tuple(program, child_pid, (child_pid != -1)));
	}		
    else {
	  std::vector<std::string> arg_split = neroshop::string::split(argument, " ");
	  char * arg_list[arg_split.size()];
	  for(int i = 0; i < arg_split.size(); i++)
	  {
		  arg_list[i] = static_cast<char *>(const_cast<char *>(arg_split[i].c_str()));
		  //std::cout << arg_list[i] << std::endl;
	  }
	  arg_list[arg_split.size()] = nullptr; // arg_list must end with a nullptr
    /* Now execute PROGRAM, searching for it in the path.  */ 
    execvp (static_cast<char *>(const_cast<char *>(program.c_str())), arg_list); 
    /* The execvp  function returns only if an error occurs.  */ 
    fprintf (stderr,  "an error occurred in execvp\n"); 
    abort (); 
  } 	
#endif
	return true;
}
////////////////////
bool neroshop::Process::terminate()
{
#ifdef _WIN32
	return (TerminateProcess(static_cast<HANDLE>(this->handle), 0) != 0);
#endif	
#ifdef __gnu_linux__
    if(handle == -1) return true; // if pid has already been killed then no need to kill it again, so exit function
    // kill function doesn't even work -.-    
    if(kill(static_cast<pid_t>(handle), SIGKILL) != 0) {// 0=success, -1=failure // #include <signal.h>
        std::cout << "FAILED to kill process: " << handle << std::endl;
        return false;
    }
    // just to be sure process has been killed
    // this doesn't work either LOL
    //std::system(std::string("kill " + std::to_string(handle)).c_str());    
    handle = -1;// set handle to default value so we know its been properly deleted
    ////std::cout << DOKUN_UI_TAG "process (" << name << ") terminated\n";
    return true;
#endif	
    return false;
}
////////////////////
bool neroshop::Process::terminate(const Process& process)
{
#ifdef _WIN32
	return (TerminateProcess(static_cast<HANDLE>(process.get_handle()), 0) != 0);
#endif
#ifdef __gnu_linux__
    // kill function doesn't even work -.-
    return const_cast<Process&>(process).terminate();//return (kill(static_cast<pid_t>(process.get_handle()), SIGTERM) != -1); //0=success, -1=failure// #include <signal.h>
#endif	
    return false;
}
////////////////////
void neroshop::Process::terminate_by_process_id(int process_id) {
#ifdef __gnu_linux__
    while(process_id != -1) {
        if(kill(static_cast<pid_t>(process_id), SIGTERM) < 0) // kill all instances of this process    
            std::cout << "FAILED to kill process " << process_id << std::endl;        
    }
#endif    
}
////////////////////
void neroshop::Process::terminate_by_process_name(const std::string& process_name) {
#ifdef __gnu_linux__    
    while(neroshop::Process::get_process_by_name(process_name) != -1) {// while this process is still running
        if(kill(static_cast<pid_t>(neroshop::Process::get_process_by_name(process_name)), SIGTERM) < 0) // kill all instances of this process    
            std::cout << "FAILED to kill process " << process_name << std::endl;
    }
#endif
}
////////////////////
void neroshop::Process::exit(int code)
{
#ifdef _WIN32
	ExitProcess(code);
#endif	
#ifdef __gnu_linux__
#endif	
}
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Process::show_processes(void) { // displays all processes from current session
    for(int i = 0; i < process_list.size(); i++) {
        std::cout 
        << "\033[1;35;49mprocess[" << i << "] ("
        << "name: " << std::get<0>(process_list[i]) << ", "
        << "id: " << std::get<1>(process_list[i]) << ", "
        << "status: " << std::get<2>(process_list[i])
        << ")\033[0m"
        << std::endl;
    }
}
////////////////////
////////////////////
////////////////////
////////////////////
#ifdef _WIN32
void * neroshop::Process::get_handle() const
{
	return this->handle;
}
#endif
////////////////////
#ifdef __gnu_linux__
int neroshop::Process::get_handle() const
{
	return handle;
}
////////////////////
std::string neroshop::Process::get_name() const {
    return name;
}
////////////////////
int neroshop::Process::get_process_by_name(const std::string& process_name) { // UPDATE(2022-02-05): this doesn't work as well as I expected it to :/
    int pid = -1;
    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                std::string cmdPath = std::string("/proc/") + dirp->d_name + "/cmdline";
                std::ifstream cmdFile(cmdPath.c_str());
                std::string cmdLine;
                std::getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (process_name == cmdLine)
                        pid = id;
                }
            }
        }
    }
    closedir(dp);
    return pid;
}
#endif
////////////////////
void * neroshop::Process::get_active()
{
#ifdef _WIN32
	return static_cast<void *>(GetCurrentProcess());
#endif
#ifdef __gnu_linux__
#endif	
    return nullptr;
}
