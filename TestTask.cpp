#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

namespace TestTask
{
	enum AccessType { // ��������� �����
		READONLY, // ���� ������ � ������ ������
		WRITEONLY, // ���� ������ � ������ ������
		CLOSED // ���� � ������ ������ �������� � ��� ����� �������
	};

	struct File {
		string fileName; // ��� �����
		map<string, File> children; // map, ������� ������ ����� �����. ���� � ��� �����, �������� ����������� �� ����
		string fileContent; // ���������� �����

		AccessType accessType; // ������ ��� ������ ���� � ������ ������.
		mutex *mut; // ��������� �� mutex, ��� ������ ����� �� ����� ���������������. �� ����, ���� ���� ����� �������������� ������ ����� �������

		File(string name) {
			fileName = name;
			fileContent = "";
			accessType = CLOSED;
			mut = new mutex();
		}

		File(string name, string content) {
			fileName = name;
			fileContent = content;
			accessType = CLOSED;

			mut = new mutex();
		}
	}; 

	struct IVFS
	{
		virtual File* Open(const char* name) = 0; // ������� ���� � readonly ������. ���� ��� ������ ����� ��� �� �� ������ �� writeonly ������ - ������� nullptr
		virtual File* Create(const char* name) = 0; // ������� ��� ������� ���� � writeonly ������. ���� �����, �� ������� ��� ������ �������������, ���������� � ����. ������� nullptr, ���� ���� ���� ��� ������ � readonly ������.
		virtual size_t Read(File* f, char* buff, size_t len) = 0; // ��������� ������ �� �����. ������������ �������� - ������� ������� ���� ������� ���������
		virtual size_t Write(File* f, char* buff, size_t len) = 0; // �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
		virtual void Close(File* f) = 0; // ������� ����	
	};

	struct MyVFS : IVFS
	{
		File root = File("root");

		string GetNextPartOfPath(string path) { // ��������������� ������� ��� �������� �����
			for (int i = 0; i < path.length(); i++) {
				if (path[i] == '\\')
				{
					return path.substr(0, i);
				}
			}
			return path;
		}

		File* Open(const char* name) { 
			// ������� ���� � readonly ������. ���� ��� ������ ����� ��� �� �� ������ �� writeonly ������ - ������� nullptr
			string path = string(name); // � ������� �� ����, ��� ���� ����� ������ "���������� 1\���������� 2\..\���������� N\����"
			File* currentFile = &root; // ����������, � ������� ��������� �������� � ������ ������

			string nextPart;
			map<string, File>::iterator it;

			while (true) {
				// ��������� �� ���� �������� ���������� � �����
				nextPart = GetNextPartOfPath(path);

				if (path.length() != nextPart.length()) { 
					// ���� ��� ��� ����������
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // �������� ����� ��� ����������
					if (it == currentFile->children.end()) { 
						// ���� � �� ����������, �� ���������� nullptr
						return nullptr;
					}
					else { 
						// ���� ��� ����������, ������� � ��
						currentFile = &(it->second);
					}
				}
				else { 
					// ���� ��� ��� ��������� �����
					it = currentFile->children.find(nextPart); // �������� ����� ���� ����
					if (it == currentFile->children.end()) { 
						// ���� ����� ��� � ���� ���������� ���������� nullptr
						cout << "there's no such file\n";
						return nullptr;
					}
					else if (it->second.accessType == WRITEONLY) {
						// ���� ���� � ������ writeonly, ���������� nullptr
						cout << "File is WRITEONLY\n";
						return nullptr;
					}
					else {
						// ���� ���� ���������� � �� ��������� � ������ readonly ��� ������ 
						File* resFile = &it->second;
						resFile->mut->lock();
						resFile->accessType = READONLY; // �� ��������� ���� � readonly
						resFile->mut->unlock();
						return resFile; // � ���������� ��������� �� ����
					}
				}
			}
		} 

		File* Create(const char* name){ 
			// ������� ��� ������� ���� � writeonly ������. ���� �����, �� ������� ��� ������ �������������, ���������� � ����. ������� nullptr, ���� ���� ���� ��� ������ � readonly ������.
			string path = string(name); // � ������� �� ����, ��� ���� ����� ������ "���������� 1\���������� 2\..\���������� N\����"
			File* currentFile = &root; // ����������, � ������� ��������� �������� � ������ ������

			string nextPart;
			map<string, File>::iterator it;

			while (true) {
				// ��������� �� ���� �������� ���������� � �����
				nextPart = GetNextPartOfPath(path);

				if (path.length() != nextPart.length()) {
					// ���� ��� ��� ����������
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // �������� ����� ��� ����������
					if (it == currentFile->children.end()) {
						// ���� � �� ����������, �� ������ � � ������� � ��.
						currentFile->children.insert(pair<string, File>(nextPart, File(nextPart)));
						currentFile = &currentFile->children.find(nextPart)->second;
					}
					else {
						// ���� ��� ����������, ������� � ��
						currentFile = &(it->second);
					}
				}
				else {
					// ���� ��� ��� ��������� �����
					it = currentFile->children.find(nextPart); // �������� ����� ���� ����
					if (it != currentFile->children.end()) {
						if (it->second.accessType == READONLY) {
							// ���� ���� ���� � ���� ���������� � �� readonly, ���������� nullptr
							return nullptr;
						}
						else {
							// ���� ���� ���������� � ���� ���������� � �� �� readonly, ��������� ��� � writeonly
							it->second.mut->lock();
							it->second.accessType = WRITEONLY;
							it->second.mut->unlock();
							// ���������� ��������� �� ����
							return &it->second;
						}
						
					}
					else {
						// ���� ����� �� ����������, �� ������ ��� � ��������� � ����� writeonly
						File resFile = File(nextPart);
						resFile.accessType = WRITEONLY;
						currentFile->children.insert(pair<string, File>(nextPart, resFile));
						return &currentFile->children.find(nextPart)->second;
					}
				}
			}
		}

		size_t Read(File* f, char* buff, size_t len) {
			// ��������� ������ �� �����. ������������ �������� - ������� ������� ���� ������� ���������
			cout << "Read file " << f->fileName << endl;
			f->mut->lock();
			cout << "start Read file " << f->fileName << endl;
			if (f->accessType == READONLY) { //���� ���� ������ ��� ������
				
				// ���� len ������ ����� ������ ��� ���� � ���� �������� ������������� �����, �� ��������� �������� len �� ����� ������.
				len = min(len, string(buff).length()); 

				// ������ len �������� � ���������� �� � buff, ���� �����
				File tmpFile = *f;
				char* fileToRead = &tmpFile.fileContent[0];
				char* start = buff;
				for (size_t i = 0; i < len; i++) {
					char charToRead = *fileToRead;
					*buff = charToRead;
					buff++;
					fileToRead++;
				}
				if (len < string(buff).length()) { // ���� ����� ������ ������ len, �� ������ ������� � ����� ������ �������
					for (size_t i = len; i < string(buff).length(); i++) {
						*buff = '\0';
						buff++;
					}
				}
				buff = start;
				cout << string(buff) << endl;
				f->mut->unlock();
				cout << "finish Read file " << f->fileName << endl;
				return size(string(buff)); // ����������, ������� ���� ������� ���������
			}
			else { // ���� ���� ������ ��� writeonly
				cout << "File is writeonly or closed\n";
				f->mut->unlock();
				cout << "finish Read file " << f->fileName << endl;
				return 0; // ���������� 0 (������ �� ������� ���������)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
			cout << "Write file " << f->fileName << endl;
			f->mut->lock();
			this_thread::sleep_for(chrono::seconds(4));
			cout << "Start write file " << f->fileName << endl;
			if (f->accessType == WRITEONLY) { //���� ���� ������ ��� ������
				// ���������� len �������� �� ������
				string bytesToWrite = string(buff).substr(0, len);
				f->fileContent = bytesToWrite;
				f->mut->unlock();
				cout << "Finish write file " << string(buff) << f->fileName << endl;
				return size(f->fileContent); // ����������, ������� ���� ������� �������� �� ������
			}
			else { // ���� ���� ������ ��� readonly
				f->mut->unlock();
				cout << "Finish write file " << f->fileName << endl;
				return 0; // ���������� 0 (������ �� ������� ��������)
			}
		}

		void Close(File* f) {
			// ������� ����	
			cout << "Close file " << f->fileName << endl;
			f->mut->lock();
			cout << "start Close file " << f->fileName << endl;
			f->accessType = CLOSED;
			f->mut->unlock();
			cout << "Finish Close file " << f->fileName << endl;
		}
	};

}