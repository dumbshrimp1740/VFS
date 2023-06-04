#include <string>
#include <map>
#include <iostream>

using namespace std;

namespace TestTask
{
	enum AccessType {
		READONLY, // ���� ������ � ������ ������
		WRITEONLY, // ���� ������ � ������ ������
		CLOSED // ���� � ������ ������ �������� � ��� ����� �������
	};

	struct File {
		string fileName; // ��� �����
		map<string, File> children; // map, ������� ������ ����� �����. ���� � ��� �����, �������� ����������� �� ����
		string fileContent; // ���������� �����

		AccessType accessType; // ������ ��� ������ ���� � ������ ������.

		File(string name) {
			fileName = name;
			fileContent = "";
			accessType = CLOSED;
		}

		File(string name, string content) {
			fileName = name;
			fileContent = content;
			accessType = CLOSED;
		}
	}; 

	struct IVFS
	{
		File root = File("root");

		static string GetNextPartOfPath(string path) {
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
				nextPart = IVFS::GetNextPartOfPath(path);

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
						resFile->accessType = READONLY; // �� ��������� ���� � readonly
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
				nextPart = IVFS::GetNextPartOfPath(path);

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
							File resFile = it->second;
							resFile.accessType = WRITEONLY;

							// ���������� ��������� �� ����
							return &resFile;
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
			
			/*
				���������� ���������� ��� len, ������� � ��������� ��� ����������� ����� ������.
				������� ������ ���� �������� � ���� � len �������� ������������� int (��-�� ������� �������� signed � unsigned �����)
			*/ 

			if (f->accessType == READONLY) { //���� ���� ������ ��� ������
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
					for (int i = len; i < string(buff).length(); i++) {
						*buff = '\0';
						buff++;
					}
				}
				buff = start;
				cout << string(buff) << endl;
				return size(string(buff)); // ����������, ������� ���� ������� ���������
			}
			else { // ���� ���� ������ ��� writeonly
				cout << "File is writeonly or closed\n";
				return 0; // ���������� 0 (������ �� ������� ���������)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
			if (f->accessType == WRITEONLY) { //���� ���� ������ ��� ������
				// ���������� len �������� �� ������
				string bytesToWrite = string(buff).substr(0, len);
				f->fileContent = bytesToWrite;
				return size(f->fileContent); // ����������, ������� ���� ������� �������� �� ������
			}
			else { // ���� ���� ������ ��� readonly
				return 0; // ���������� 0 (������ �� ������� ��������)
			}
		}

		void Close(File* f) {
			// ������� ����	
			f->accessType = CLOSED;
		}
	};

}