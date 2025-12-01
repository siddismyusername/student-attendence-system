#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

using namespace std;

class Config {
public:
    static map<string, string> loadConfig(const string& filename);
    
private:
    static string trim(const string& str);
};

#endif // CONFIG_H
