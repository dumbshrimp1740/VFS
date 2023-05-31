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
						return nullptr;
					}
					else if (it->second.accessType == WRITEONLY) {
						// ���� ���� � ������ writeonly, ���������� nullptr
						return nullptr;
					}
					else {
						// ���� ���� ���������� � �� ��������� � ������ readonly ��� ������ 
						File resFile = it->second;
						resFile.accessType = READONLY; // �� ��������� ���� � readonly
						return &resFile; // � ���������� ��������� �� ����
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
			if (f->accessType == READONLY) { //���� ���� ������ ��� ������
				// ������ len ��������
				string readBytes = f->fileContent.substr(0, min(len, f->fileContent.length())); 
				buff = &readBytes[0];
				return sizeof(buff); // ����������, ������� ���� ������� ���������
			}
			else { // ���� ���� ������ ��� writeonly
				return 0; // ���������� 0 (������ �� ������� ���������)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
			if (f->accessType == WRITEONLY) { //���� ���� ������ ��� ������
				// ���������� len �������� �� ������
				string bytesToWrite = string(buff).substr(0, min(len, string(buff).length()));
				f->fileContent.append(bytesToWrite);
				return sizeof(bytesToWrite); // ����������, ������� ���� ������� �������� �� ������
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