#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <future>
#include <mutex> 
#include <io.h>
#include <regex>
#include <windows.h>
#include <atlstr.h>
using namespace std;

void regex_match_first(string_view file, string_view filepath, const string& pattern);
void findAllSubDir(const string srcpath, string_view suffix, const string& pattern);
void findAllSubDir_AllFile(const string srcpath, const string& pattern);
string readFile(const char* filename);
bool opentxt(void);
