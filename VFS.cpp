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
    MyVFS myVFS = MyVFS();
    File* root = &myVFS.root;

    File* resfile = myVFS.Create("file1");
    File* resfile2 = myVFS.Create("file2");
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

    thread thr(&MyVFS::Write, myVFS, resfile, buf2, 16);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr2(&MyVFS::Write, myVFS, resfile, buf1, 5);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr4(&MyVFS::Write, myVFS, resfile, buf4, 8);
    this_thread::sleep_for(chrono::milliseconds(10));
    thread thr8(&MyVFS::Write, myVFS, resfile2, buf1, 8);
    this_thread::sleep_for(chrono::milliseconds(200));
    myVFS.Close(resfile);
    myVFS.Open("file1");
    myVFS.Close(resfile2);
    myVFS.Open("file2");

    thread thr3(&MyVFS::Read, myVFS, resfile, buf3, 13);
    thr3.join();
    thread thr7(&MyVFS::Read, myVFS, resfile2, buf7, 14);
    thr7.join();
    cout << string(buf3) << " " << string(buf7) << endl;;
    thr.join();
    thr2.join();
    thr4.join();
    thr8.join();
}


