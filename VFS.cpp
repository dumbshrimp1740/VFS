#include <iostream>
#include <string>
#include "TestTask.cpp"

using namespace std;
using namespace TestTask;

int main()
{
    //Ôאיכ הכÿ עוסעמג
    MyVFS myVFS = MyVFS();
    File* root = &myVFS.root;

    File* resfile = myVFS.Create("file1\\file12");
    myVFS.Create("file1\\file13");
    myVFS.Create("file1\\file14");
    myVFS.Create("file1\\file15");
    myVFS.Create("file1\\file13");
    string s_write = "ITS ME AUSTIN";
    myVFS.Write(resfile, &s_write[0], 15);
    myVFS.Close(resfile);
    myVFS.Open("file1\\file12");
    string s_read2 = "123456789012345";
    char* s_read1 = &s_read2[0];
    cout << "read " << myVFS.Read(resfile, s_read1, -5) << " bytes\n";
    cout << string(s_read1);
}


