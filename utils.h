#pragma once

#include <string>
#include <map>

using namespace std;

string readFile(const string& filename);

string renderTemplate(const string& filename, map<string, string>& data);

string renderTemplate(const string& filename);