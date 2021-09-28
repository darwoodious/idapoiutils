#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/wait.h>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Not enough parameters" << endl;
    return 1;
  }

  string ssid = argv[1];
  string password;

  if (argc > 2)
  {
    password = argv[2];
  }

  cerr << "SSID: " << ssid << endl;
  cerr << "Password: " << password << endl;

  int stdInPipePair[2];

  if (pipe(stdInPipePair) < 0)
  {
    cerr << "Failed to create pipe pair" << endl;
    return 1;
  }

  int stdInPipeParent = stdInPipePair[1];
  int stdInPipeChild = stdInPipePair[0];

  int pid = fork();

  if (pid < 0)
  {
    cerr << "Fork() failed" << endl;
    return pid;
  }

  if (pid > 0)
  {
    // Parent.
    close(stdInPipeChild);

    uint8_t lengthBuffer[1];

    lengthBuffer[0] = (uint8_t) ssid.size();

    write(stdInPipeParent, lengthBuffer, 1);
    write(stdInPipeParent, ssid.data(), ssid.size());

    lengthBuffer[0] = (uint8_t) password.size();

    write(stdInPipeParent, lengthBuffer, 1);
    write(stdInPipeParent, password.data(), password.size());

    close(stdInPipeParent);

    int processStatus;

    waitpid(pid, &processStatus, 0);

    return processStatus;
  }
  else
  {
    // Child;
    close(stdInPipeParent);

    if (dup2(stdInPipeChild, STDIN_FILENO) == -1) {
      cerr << "<child> dup2 failed" << endl;
      exit(errno);
    }

    execvp("./idasetwifi", nullptr);
    cerr << "<child> execvp failed" << endl;
    exit(errno);
  }
}
