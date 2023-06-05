#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "TestTask.cpp"

using namespace std;
using namespace TestTask;

int main()
{
    //Ôאיכ הכÿ עוסעמג
    MyFS myFS = MyFS();
    File* root = myFS.Open("root");

    File* resfile = myFS.Create("root\\file1");
    File* resfile2 = myFS.Create("root\\file2");
    File* resfile3 = myFS.Create("root\\\\\\\\");

    string str1 = "it's me austin!";
    char* buf1 = &str1[0];

    string str2 = "deadman walking";
    char* buf2 = &str2[0];

    string str4 = "crabobaabobovna";
    char* buf4 = &str4[0];


    string str3 = "11111111111111111111111111111";
    char* buf3 = &str3[0];
    string str7 = "11111111111111111111111111111";
    char* buf7 = &str7[0];

    thread thr(&MyFS::Write, myFS, resfile, buf2, 16);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr2(&MyFS::Write, myFS, resfile, buf1, 5);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr4(&MyFS::Write, myFS, resfile, buf4, 8);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr8(&MyFS::Write, myFS, resfile2, buf1, 8);
    this_thread::sleep_for(chrono::milliseconds(200));
    myFS.Close(resfile);
    myFS.Open("root\\file1");
    myFS.Close(resfile2);
    myFS.Open("root\\file2");

    thread thr3(&MyFS::Read, myFS, resfile, buf3, 13);
    thr3.join();
    thread thr7(&MyFS::Read, myFS, resfile2, buf7, 14);
    thr7.join();
    cout << string(buf3) << " " << string(buf7) << endl;;
    thr.join();
    thr2.join();
    thr4.join();
    thr8.join();
    cout << root->children.size() << endl;
    for (auto it = root->children.begin(); it != root->children.end(); it++) {
        cout << it->second.fileName << endl;
    }
    if (resfile3 != nullptr)
        cout << resfile3->fileName;
    else
        cout << "oops";
}


