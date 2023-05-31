#include <iostream>
#include <string>
#include "TestTask.cpp"

using namespace std;
using namespace TestTask;

int main()
{
    string path = "root\\tworoot";
    File root = File("root");
    IVFS ivfs = IVFS();

    File* resfile = ivfs.Create("root\\tworoot");
    string s = "ITS ME AUSTIN";
    string res = "";
    ivfs.Write(resfile, &s[0], 100);
    ivfs.Close(resfile);
    File* secondFile = ivfs.Open("root\\tworoot");
    for (auto it = ivfs.root.children.find("root")->second.children.begin(); it != ivfs.root.children.find("root")->second.children.end(); it++) {
        cout << it->second.fileContent;
    }
}


