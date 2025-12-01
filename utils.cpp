#include "utils.h"

#include <fstream>
#include <sstream>
#include <iostream>

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть " << filename << endl;
        return "<h1>Ошибка: файл не найден</h1>";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string renderTemplate(const string& filename, map<string, string>& data) {
    string content = readFile("templates/" + filename);
    
    for (const auto& [key, value] : data) {
        string placeholder = "{{" + key + "}}";
        size_t pos = content.find(placeholder);
        while (pos != string::npos) {
            content.replace(pos, placeholder.length(), value);
            pos = content.find(placeholder, pos + value.length());
        }
    }
    return content;
}

string renderTemplate(const string& filename) {
    map<string, string> empty;
    return renderTemplate(filename, empty);
}