#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <list>

using namespace std;

// some helper functions
const string whitespace = " \t";
const string quote = "\"";  
const string comma = ",";  

string trim(const string& str)
{
  const auto str_begin = str.find_first_not_of(whitespace);
  if (str_begin == string::npos) return ""; // no content

  const auto str_end = str.find_last_not_of(whitespace);
  const auto range = str_end - str_begin + 1;

  return str.substr(str_begin, range);
}

bool stringStartsWith(const string& str, const string& phrase)
{
  return str.rfind(phrase, 0) == 0;
}

string getQuality(const string& str)
{
  const auto equals_pos = str.find_first_of("=");
  const auto div_pos = str.find_first_of("/");
  const auto first_space = str.find_first_of(whitespace);
  if (string::npos == equals_pos || string::npos == div_pos || string::npos == first_space) return ""; // no content

  string numerator_str = str.substr(equals_pos+1, (div_pos-equals_pos+1));
  string denominator_str = str.substr(div_pos+1, (first_space-div_pos+1));

  int numerator = std::stoi(numerator_str) * 100;
  int denominator = std::stoi(denominator_str);
  int pct = numerator/denominator;

  return to_string(pct);
}

string hasEncryption(const string& str)
{
  const auto colon_pos = str.find_first_of(":");
  if (string::npos == colon_pos) return ""; // no content

  string enc_str = str.substr(colon_pos+1);
  if (enc_str == "on") return "y";
  if (enc_str == "off") return "n";
  return "?";
}

string getSSID(const string& str)
{
  const auto colon_pos = str.find_first_of(":");
  const auto first_quote = str.find_first_of('"', colon_pos);
  const auto last_quote = str.find_last_of('"');
  if (string::npos == colon_pos || string::npos == first_quote || string::npos == last_quote) return ""; // no content

  return str.substr(first_quote+1, (last_quote-first_quote-1));
}


int main(void) 
{
  // filenames and stuff
  char iwlist_filename[32];
  char iwlist_cmd[128];
  char iwgetid_filename[32];
  char iwgetid_cmd[128];
  sprintf(iwlist_filename, "/tmp/iwlist.%i", getpid());
  sprintf(iwlist_cmd, "/usr/sbin/iwlist wlan0 scan > %s", iwlist_filename);
  sprintf(iwgetid_filename, "/tmp/iwgetid.%i", getpid());
  sprintf(iwgetid_cmd, "/usr/sbin/iwgetid --raw > %s", iwgetid_filename);

  int HR=0;
  string current_ssid;
  list<string> ssid_list;

  // let's be root
  setuid(0);

  // first let's see if we're already connected to a network
  HR = system(iwgetid_cmd);
  if(HR)
  {
    // didn't work but we'll just set the current SSID to "" and it won't match
    current_ssid = "";
  }
  else
  {
    ifstream ssidfile(iwgetid_filename);
    if (ssidfile.is_open())
    {
      getline(ssidfile, current_ssid);
      ssidfile.close();
      remove(iwgetid_filename);
    }
  }

  // attempt to get the content from iwlist
  HR = system(iwlist_cmd);
  if(HR)
  {
    // that didn't work...
    cerr << "problem issuing the iwlist call. exiting." << endl;
    return(HR); 
  }
  
  // open up our datafile and parse
  ifstream datafile(iwlist_filename);
  if (datafile.is_open())
  {
      bool in_cell = false;
      bool got_quality = false;
      bool got_encryption = false;
      bool got_ssid = false;
      string quality = "";
      string encrypted = "";
      string ssid = "";
      string line;

      while(getline(datafile, line))
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
          if (stringStartsWith(data, "Cell "))
          {
            in_cell = true;
            /*
            cout << "CELL       " 
                 << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
                 << endl; // << data << endl;
            */
            continue;
          }
          if (stringStartsWith(data, "Quality="))
          {
            got_quality = true;
            quality = getQuality(data);
            /*
            cout << "QUALITY    " 
                 << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
                 << endl << data << endl;
            cout << "Quality: " << quality << "%" << endl;
            */
            continue;
          }
          if (stringStartsWith(data, "Encryption key:"))
          {
            got_encryption = true;
            encrypted = hasEncryption(data);
            /*
            cout << "ENCRYPTION " 
                 << "[" << (in_cell?"C":" ") << (got_quality?"Q":" ") << (got_encryption?"E":" ") << (got_ssid?"S":" ") << "]"
                 << endl << data << endl;
            cout << "Encrypted: " << encrypted << endl;
            */
            continue;
          }
          if (stringStartsWith(data, "ESSID:"))
          {
            got_ssid = true;
            ssid = getSSID(data);
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
  }
  else {
      cerr << "Couldn't open config file for reading.\n";
  }
  // cleanup
  remove(iwlist_filename);
  exit(HR);
}


