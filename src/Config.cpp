#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

map<string, string> Config::loadConfig(const string& filename) {
    map<string, string> config;
    ifstream file(filename);
    
    if (!file.is_open()) {
        return config;
    }
    
    string line;
    while (getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != string::npos) {
            string key = trim(line.substr(0, pos));
            string value = trim(line.substr(pos + 1));
            config[key] = value;
        }
    }
    
    file.close();
    return config;
}

string Config::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
