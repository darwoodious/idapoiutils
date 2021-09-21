#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;
int main(int argc, char *argv[]) 
{
  // filenames and stuff
  const char* conf_name = "/etc/wpa_supplicant/wpa_supplicant.conf";
  const char* conf_swap_name = "/etc/wpa_supplicant/wpa_supplicant.swap";
  char conf_backup_name[512];
  sprintf(conf_backup_name, "/etc/wpa_supplicant/wpa_supplicant.conf.%i", getpid());
  const char* wpi_command = "/usr/sbin/wpa_cli -i wlan0 reconfigure";
  int HR=0;

  // only thing on STDIN should be a single line with passwd. we'll ignore everything else.
  string ssid;
  string passwd;

  if (true)
  {
    getline(cin, ssid);
    getline(cin, passwd);
  }
  
  // let's be root
  setuid(0);

  // create new wpa_supplicant.conf file
  ofstream conf_file(conf_swap_name, ofstream::out);
  conf_file << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" << endl
            << "update_config=1" << endl << "network={" << endl
            << "\tssid=\"" << ssid << "\"" << endl
            << "\tpsk=\"" << passwd << "\"" << endl << "}" << endl;
  conf_file.close();

  // now move the old config file to a backup and swap in the new one
  HR = rename(conf_name, conf_backup_name);
  if(HR)
  {
    cerr << "error backing up old config file: " << conf_name << " to: " << conf_backup_name << endl;
    return(HR); 
  }
  HR = rename(conf_swap_name, conf_name);
  if(HR)
  {
    cerr << "error swapping new config file: " << conf_swap_name << " with: " << conf_name << endl
         << "wifi should be unaffected." << endl;
    return(HR); 
  }

  // should be ready to go... load it into the system
  HR = system(wpi_command);
  if(HR) 
  {
    // ok, problem - something didn't work. let's try to backout
    int HRX = rename(conf_backup_name, conf_name);

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
  remove(conf_swap_name);
  
  return(HR);
}


