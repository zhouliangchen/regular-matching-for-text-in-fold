#include "state.h"
ofstream fout;

void regex_match_first(string_view file, string_view filepath, const string& pattern) {
    regex REG(pattern);
    size_t line = 1;
    static mutex mtx;
    for (regex_iterator<string_view::const_iterator> iter(file.cbegin(), file.cend(), REG), end_it; iter != end_it; ++iter) {
        auto pos = iter->prefix().length();
        //cout << iter->prefix().str() << endl;
        string&& prestr = iter->prefix().str();
        line += count(prestr.begin(), prestr.end(), '\n');
        pos = pos > 50 ? pos - 50 : 0;
        mtx.lock();
        fout << "文件路径：" << filepath << "  line：" << line << endl;
        fout << iter->prefix().str().substr(pos) << "\n\t\t>>>" << iter->str() << "<<<\n" << iter->suffix().str().substr(0, 50) << endl << endl << endl; /*输出匹配字符串及其前后50个字符*/
        mtx.unlock();
    }
    return;
}
void findAllSubDir(const string srcpath, string_view suffix, const string& pattern) {
    _finddata_t file;
    intptr_t lf;
    string  curpath; //当前路径
    //vector<thread> tp;
    vector<future<void>>tp; //获取异步操作的结果
    if ((lf = _findfirst(curpath.assign(srcpath).append("\\*").c_str(), &file)) == -1l)
        //_findfirst返回的是long型;long __cdecl _findfirst(const char *, struct _finddata_t *)  
        return;
    else {
        do {
            curpath = file.name;
            if (curpath != "." && curpath != "..") {
                if (file.attrib == _A_SUBDIR) {
                    curpath = srcpath + "\\" + curpath;
                    tp.emplace_back(async(launch::async, findAllSubDir, curpath, suffix, pattern));
                }
                else if (!curpath.substr(curpath.rfind(".") + 1).compare(suffix)) {
                    string&& filepath = srcpath + "\\" + curpath;
                    string&& tempfile = readFile(filepath.c_str());
                    regex_match_first(tempfile, filepath, pattern);
                }
            }
        } while (_findnext(lf, &file) == 0);
        //int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1  
    }
    _findclose(lf);
    for (auto& p : tp) {
        p.get();
    }
}
void findAllSubDir_AllFile(const string srcpath, const string& pattern) {
    _finddata_t file;
    intptr_t lf;
    string  curpath; //当前路径
    //vector<thread> tp;
    vector<future<void>>tp; //获取异步操作的结果
    if ((lf = _findfirst(curpath.assign(srcpath).append("\\*").c_str(), &file)) == -1l)
        //_findfirst返回的是long型;long __cdecl _findfirst(const char *, struct _finddata_t *)  
        return;
    else {
        do {
            curpath = file.name;
            if (curpath != "." && curpath != "..") {
                if (file.attrib == _A_SUBDIR) {
                    curpath = srcpath + "\\" + curpath;
                    tp.emplace_back(async(launch::async, findAllSubDir_AllFile, curpath, pattern));
                }
                else {
                    string&& filepath = srcpath + "\\" + curpath;
                    string&& tempfile = readFile(filepath.c_str());
                    regex_match_first(tempfile, filepath, pattern);
                }
            }
        } while (_findnext(lf, &file) == 0);
        //int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1  
    }
    _findclose(lf);
    for (auto& p : tp) {
        p.get();
    }
}
string readFile(const char* filename) {
    /*static int count = 0;
    count++;
    static mutex mtx;
    mtx.lock();
    cout << count << " " << filename << endl;
    mtx.unlock();*/
    ifstream in(filename);
    //将文件读入到ostringstream对象out中
    ostringstream out;
    out << in.rdbuf();
    return out.str();
}
bool opentxt(void) {
    LPWSTR sSysDir = new WCHAR[MAX_PATH];
    GetSystemDirectory(sSysDir, MAX_PATH);
    CString strFullPath = sSysDir;
    delete[] sSysDir;
    strFullPath += _T("\\cmd.exe");
    CString strCmdLine = _T(" /C ");
    strCmdLine += _T("notepad.exe ");
    strCmdLine += _T("匹配结果.txt");
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL bRunProgram = CreateProcess(strFullPath.GetBuffer(), strCmdLine.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    strFullPath.ReleaseBuffer();
    strCmdLine.ReleaseBuffer();
    if (!bRunProgram)
    {
        return false;
    }
    // 等待程序结束
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

int main() {
    cout << "——————测试系统菜单——————" << endl;
    cout << "1.开始查找" << endl;
    cout << "0.退出" << endl;
    int x;
    string path; //查找文件夹路径
    string suffix;  //查找的文件类型后缀
    string pattern;
    while (cin >> x, getchar(),x != 0) {
        switch (x) {
        case 1:
            cout << "暂仅支持ANSL编码" << endl;
            cout << "请输入待查找目录绝对路径" << endl;
            getline(cin, path);
            if (_access(path.c_str(), 0) != 0) {
                cout << "目标路径不存在，请检查" << endl;
                break;
            }
            cout << "请输入待查找文件后缀（所有文件则直接回车）" << endl;
            getline(cin, suffix);
            cout << "请输入待匹配正则表达式" << endl;
            getline(cin, pattern);

            fout.open("匹配结果.txt", ios_base::out | ios_base::trunc);
            if (!fout.is_open()) {
                cout << "无法打开输出文件 " << endl;
                break;
            }
            cout << "正在进行查找......" << endl;
            if(suffix.empty())findAllSubDir_AllFile(path, pattern);
            else findAllSubDir(path, suffix, pattern);
            cout << "扫描完毕" << endl;
            if (opentxt())cout << "打开输出文件" << endl;
            else cout << "系统调用异常" << endl;
            fout.close();
            break;
        default:
            cout << "输入错误，请重试" << endl;
        }
        system("pause");
        system("cls");
        cout << "——————测试系统菜单——————" << endl;
        cout << "1.开始查找" << endl;
        cout << "0.退出" << endl;
    }
    return 0;
}