#include <iostream>
#include <string>
#include "TestTask.cpp"

using namespace std;
using namespace TestTask;

int main()
{
    //Ôאיכ הכÿ עוסעמג
    IVFS ivfs = IVFS();
    File* root = &ivfs.root;

    File* resfile = ivfs.Create("file1\\file12");
    ivfs.Create("file1\\file13");
    ivfs.Create("file1\\file14");
    ivfs.Create("file1\\file15");
    ivfs.Create("file1\\file13");
    string s_write = "ITS ME AUSTIN";
    ivfs.Write(resfile, &s_write[0], 15);
    ivfs.Close(resfile);
    ivfs.Open("file1\\file12");
    string s_read2 = "123456789012345";
    char* s_read1 = &s_read2[0];
    cout << "read " << ivfs.Read(resfile, s_read1, 18) << " bytes\n";
    cout << string(s_read1);
}


