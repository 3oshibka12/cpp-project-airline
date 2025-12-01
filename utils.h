#pragma once

#include <string>
#include <map>

using namespace std;

// Чтение файла в строку
string readFile(const string& filename);

// Шаблонизатор
string renderTemplate(const string& filename, map<string, string>& data);

// Перегрузка без данных (для простых страниц)
string renderTemplate(const string& filename);