#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
// void * runner(void *)
// {
//     return;
// }
char errMsg[] = "./mygrader [num_of_threads]";

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cerr << errMsg << std::endl;
        exit(EXIT_FAILURE);
    }

    int numThreads = std::stoi(argv[1]);
    if (numThreads <= 0)
    {
        std::cerr << errMsg << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Number of threads: " << numThreads << std::endl;
    int fd = open("./input/1.in", O_RDONLY);
    if (fd == -1)
    {
        std::cerr << "Error opening input file\n";
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDIN_FILENO) < 0)
    {
        std::cerr << "Error dup\n";
        exit(EXIT_FAILURE);
    }

    if (execlp("./build/client", "./build/client", "3000", NULL) < 0)
    {
        std::cerr << "Error running client\n";
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}