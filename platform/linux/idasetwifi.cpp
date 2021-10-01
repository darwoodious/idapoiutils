#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include "Types.h"
#include "AsciiEncoder.h"
#include "HexConverter.h"
#include "Process.h"

using namespace std;
using namespace Sequent;

uint8_t ReadByte(FILE *stream);
void ReadExact(uint8_t *buffer , size_t count, FILE *stream);
string BinaryToAsciiOrHex(uint8_vector &binary, bool &isAscii);

int main(int argc, char *argv[])
{
  // filenames and stuff
  string prefix("/etc/wpa_supplicant/");
  const string conf_name = prefix + "wpa_supplicant.conf";
  const string conf_swap_name = prefix + "wpa_supplicant.swap";
  const string conf_backup_name = prefix + "wpa_supplicant.conf." + to_string(getpid());
  int HR=0;
  uint8_t byteCount;

  try
  {
    byteCount = ReadByte(stdin);
  }
  catch (const char *e)
  {
    cerr << "Failed to read SSID byte count from stdin" << endl;
    cerr << "  " << e << endl;
    return 1;
  }

  if (byteCount < 1 || byteCount > 32)
  {
    cerr << "SSID byte count invalid" << endl;
    return 1;
  }

  uint8_vector ssid(byteCount);

  try
  {
    ReadExact(ssid.data(), byteCount, stdin);
  }
  catch (const char *e)
  {
    cerr << "Failed to read SSID from stdin" << endl;
    cerr << "  " << e << endl;
    return 1;
  }

  try
  {
    byteCount = ReadByte(stdin);
  }
  catch (const char *e)
  {
    cerr << "Failed to read Password byte count from stdin" << endl;
    cerr << "  " << e << endl;
    return 1;
  }

  if (byteCount > 100)
  {
    cerr << "Password byte count invalid" << endl;
    return 1;
  }

  uint8_vector password(byteCount);

  try
  {
    ReadExact(password.data(), byteCount, stdin);
  }
  catch (const char *e)
  {
    cerr << "Failed to read Password from stdin" << endl;
    cerr << "  " << e << endl;
    return 1;
  }

  bool ssidIsAscii;
  string ssidAscii = BinaryToAsciiOrHex(ssid, ssidIsAscii);
  bool passwordIsAscii;
  string passwordAscii = BinaryToAsciiOrHex(password, passwordIsAscii);

  // let's be root
  setuid(0);

  // create new wpa_supplicant.conf file
  ofstream conf_file(conf_swap_name, ofstream::out);
  conf_file << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" << endl
            << "update_config=1" << endl
            << "country=US" << endl << endl
            << "network={" << endl
            << "\tssid=" << (ssidIsAscii ? "\"" : "") << ssidAscii << (ssidIsAscii ? "\"" : "") << endl
            << "\tpsk=" << (passwordIsAscii ? "\"" : "") << passwordAscii << (passwordIsAscii ? "\"" : "") << endl
            << "}" << endl;
  conf_file.close();

  if(access(conf_name.c_str(), F_OK) == 0) {
    // now move the old config file to a backup and swap in the new one
    HR = rename(conf_name.c_str(), conf_backup_name.c_str());

    if(HR)
    {
      cerr << "error backing up old config file: " << conf_name << " to: " << conf_backup_name << endl;
      return(HR);
    }
  }

  HR = rename(conf_swap_name.c_str(), conf_name.c_str());

  if(HR)
  {
    cerr << "error swapping new config file: " << conf_swap_name << " with: " << conf_name << endl
         << "wifi should be unaffected." << endl;
    return(HR);
  }

  uint8_vector standardError;
  unique_ptr<Process> process;

  // should be ready to go... load it into the system
  process = Process::Execute("/usr/sbin/wpa_cli", Redirect::StandardError, "wpa_cli", "-i", "wlan0", "reconfigure");

  process->ExchangeStandardIo(nullptr, nullptr, &standardError);
  process->Wait();

  if (standardError.size())
  {
    cerr << AsciiEncoder::Decode(standardError);

    if (standardError.back() != '\n')
    {
      cerr << endl;
    }
  }

  if(process->GetExitStatus())
  {
    // ok, problem - something didn't work. let's try to backout
    int HRX = rename(conf_backup_name.c_str(), conf_name.c_str());

    // need to return initial error from the wpi command. our STDERR response will depend on the backout too.
    if(HRX)
    {
      cerr << "activating new wifi failed. also, the restore failed." << endl;
    }
    else
    {
      // backout should have worked...
      cerr << "activating new wifi failed. restored to original value." << endl;
    }
  }

  //cleanup and leave
  remove(conf_swap_name.c_str());

  return(HR);
}

uint8_t ReadByte(FILE *stream)
{
  int byte = fgetc(stream);

  if (byte < 0)
  {
    throw "Unexpected End of File";
  }

  return (uint8_t) byte;
}

void ReadExact(uint8_t *buffer, size_t count, FILE *stream)
{
  size_t offset = 0;

  while (count > 0)
  {
    int bytesRead = fread(buffer + offset, 1, count, stream);

    if (ferror(stream))
    {
      throw "File read error";
    }

    if (bytesRead == 1)
    {
      throw "Unexpected End of File";
    }

    count -= bytesRead;
    offset += bytesRead;
  }
}

string BinaryToAsciiOrHex(uint8_vector &binary, bool &isAscii)
{
  string string;

  isAscii = true;

  for (uint8_vector::const_iterator it = binary.cbegin(); it != binary.cend(); ++it)
  {
    if (*it < 0x20 || *it > 126)
    {
      isAscii = false;
      break;
    }

    string += (char) *it;
  }

  if (isAscii)
  {
    return string;
  }
  else
  {
    return HexConverter::ByteArrayToHexString(binary);
  }
}
