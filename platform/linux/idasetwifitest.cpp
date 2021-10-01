#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/wait.h>

#include "Types.h"
#include "AsciiEncoder.h"
#include "Pipe.h"
#include "Process.h"

using namespace std;
using namespace Sequent;

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

  uint8_vector lengthBuffer(1);
  unique_ptr<uint8_vector> ssidBuffer = AsciiEncoder::Encode(ssid);
  unique_ptr<uint8_vector> passwordBuffer = AsciiEncoder::Encode(password);
  unique_ptr<Process> process = Process::Execute("./idasetwifi", "idasetwifi");
  Pipe &standardIn(process->GetStandardIn());

  lengthBuffer[0] = (uint8_t) ssidBuffer->size();
  standardIn.Write(lengthBuffer, 0, lengthBuffer.size());
  standardIn.Write(*ssidBuffer, 0, ssidBuffer->size());

  lengthBuffer[0] = (uint8_t) passwordBuffer->size();
  standardIn.Write(lengthBuffer, 0, lengthBuffer.size());
  standardIn.Write(*passwordBuffer, 0, passwordBuffer->size());

  standardIn.Close();
  process->Wait();

  return process->GetExitStatus();
}

