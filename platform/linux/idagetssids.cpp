#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <list>
#include <cmath>
#include <memory>
#include <sstream>

#include "AsciiEncoder.h"
#include "Pipe.h"
#include "Process.h"

using namespace std;
using namespace Sequent;

// some helper functions
const string whitespace = " \t\r\n";
const string quote = "\"";
const string comma = ",";

string trim(const string str)
{
  const auto str_begin = str.find_first_not_of(whitespace);
  if (str_begin == string::npos) return ""; // no content

  const auto str_end = str.find_last_not_of(whitespace);
  const auto range = str_end - str_begin + 1;

  return str.substr(str_begin, range);
}

bool stringStartsWith(const string str, const string phrase)
{
  return str.rfind(phrase, 0) == 0;
}

string removeFirst(const string str, size_t count)
{
  return str.substr(count, str.size() - count);
}

// Apply character escaping for the " character, in the same way
// that the iw command escaped non-printable characters.
string escapeQuotes(const string str)
{
  string escaped;

  for (string::const_iterator it = str.cbegin(); it != str.cend(); ++it)
  {
    if (*it != '"')
    {
      escaped += *it;
    }
    else
    {
      escaped += "\\x22";
    }
  }

  return escaped;
}

double between(double d, double minimum, double maximum)
{
  if (d < minimum)
  {
    return minimum;
  }
  else if (d > maximum)
  {
    return maximum;
  }
  else
  {
    return d;
  }
}

size_t getSignalStrengthPercentage(double decibels)
{
  return (size_t) lround(
    (100.0 - abs(between(decibels, -90, -30)) - 10.0) / 60.0 * 100.0
  );
}

string getSignalStrength(const string str)
{
  return to_string(
    getSignalStrengthPercentage(
      stod(str.substr(0, str.size() - 4))
    )
  );
}

string hasEncryption(const string str)
{
  return
    (str.find_first_of("Privacy") != string::npos)
    ? "y"
    : "n";
}

string getSSID(const string& str)
{
  return escapeQuotes(str);
}

int main(void)
{
  uint8_vector standardOut;
  uint8_vector standardError;
  unique_ptr<Process> process;
  string current_ssid;
  list<string> ssid_list;

  // let's be root
  setuid(0);

  process = Process::Execute("/usr/sbin/iwgetid", Redirect::Outputs, "iwgetid", "--raw");

  process->ExchangeStandardIo(nullptr, &standardOut, &standardError);
  process->Wait();

  if (standardError.size())
  {
    cerr << AsciiEncoder::Decode(standardError);

    if (standardError.back() != '\n')
    {
      cerr << endl;
    }
  }

  if (process->GetExitStatus())
  {
    // didn't work but we'll just set the current SSID to "" and it won't match
  }
  else
  {
    current_ssid = trim(AsciiEncoder::Decode(standardOut));
  }

  standardOut.clear();
  standardError.clear();

  // attempt to get the content from iw
  process = Process::Execute("/usr/sbin/iw", Redirect::Outputs, "iw", "wlan0", "scan");

  process->ExchangeStandardIo(nullptr, &standardOut, &standardError);
  process->Wait();

  if (standardError.size())
  {
    cerr << AsciiEncoder::Decode(standardError);

    if (standardError.back() != '\n')
    {
      cerr << endl;
    }
  }

  if (process->GetExitStatus())
  {
    // that didn't work...
    cerr << "problem issuing the iwlist call. exiting." << endl;
    return process->GetExitStatus();
  }

  string standardOutAscii = AsciiEncoder::Decode(standardOut);
  istringstream standardOutStream(standardOutAscii);

  bool in_cell = false;
  bool got_quality = false;
  bool got_encryption = false;
  bool got_ssid = false;
  string quality = "";
  string encrypted = "";
  string ssid = "";
  string line;

  while(getline(standardOutStream, line))
  {
    if (in_cell && got_quality && got_encryption && got_ssid)
    {
      // done with this cell. save the data and reset unless the SSID is ""
      if (!ssid.empty())
      {
        string row = quote + ssid + quote + comma + quality + comma + encrypted + comma + (ssid==current_ssid?"y":"n");
        ssid_list.push_back(row);
      }
      // reset flags
        in_cell = got_quality = got_encryption = got_ssid = false;
      }

      // process a line.
      string data = trim(line);
      if (stringStartsWith(data, "BSS "))
      {
        in_cell = true;
        /*
        cout << "CELL       "
             << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
             << endl; // << data << endl;
        */
        continue;
      }
      if (stringStartsWith(data, "signal: "))
      {
        got_quality = true;
        quality = getSignalStrength(removeFirst(data, 8));
        /*
        cout << "QUALITY    "
             << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
             << endl << data << endl;
        cout << "Quality: " << quality << "%" << endl;
        */
        continue;
      }
      if (stringStartsWith(data, "capability: "))
      {
        got_encryption = true;
        encrypted = hasEncryption(removeFirst(data, 12));
        /*
        cout << "ENCRYPTION "
             << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
             << endl << data << endl;
        cout << "Encrypted: " << encrypted << endl;
        */
        continue;
      }
      if (stringStartsWith(data, "SSID: "))
      {
        got_ssid = true;
        ssid = getSSID(removeFirst(data, 6));
        /*
        cout << "SSID       "
             << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
             << endl << data << endl;
        cout << "SSID:" << encrypted << endl;
        */
        continue;
      }
  }

  for (auto row : ssid_list)
  {
    cout << row << endl;
  }

  exit(process->GetExitStatus());
}
