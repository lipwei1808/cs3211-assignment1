#include <fstream>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>

char errMsg[] = "./mygrader [input_file_1]...\n\t example: ./mygrader ./input/1.in";

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cerr << errMsg << std::endl;
        exit(EXIT_FAILURE);
    }


    std::cout << "Number of threads: " << argc - 1 << std::endl;
    std::vector<pid_t> children(argc - 1);
    for (int i = 0; i < argc - 1; i++)
    {
        char * filename = argv[i + 1];
        int fd = open(filename, O_RDONLY);
        if (fd == -1)
        {
            std::cerr << "Error opening input file: " << filename << std::endl;
            continue;
        }
        if (dup2(fd, STDIN_FILENO) < 0)
        {
            std::cerr << "Error dup\n";
            continue;
        }
        children[i] = fork();
        switch (children[i])
        {
            case -1: {
                std::cerr << "error creating fork for input file: " << filename << std::endl;
                continue;
            }
            case 0: {
                if (execlp("./build/client", "./build/client", "3000", NULL) < 0)
                {
                    std::cerr << "Error running client\n";
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, 0)) != -1)
    {
        bool found = false;
        for (int i = 0; i < argc - 1; i++)
        {
            if (children[i] == pid)
            {
                std::cout << "status for " << argv[i + 1] << ": " << status << std::endl;
                found = true;
                break;
            }
        }
        if (!found)
            std::cout << "pid: " << pid << ", not found!\n";
    }

    return EXIT_SUCCESS;
}