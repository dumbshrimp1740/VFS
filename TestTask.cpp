#include <string>
#include <map>
#include <mutex>
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
		map<string, File> children; // map, ������� ������ ����� �����. ���� � ��� �����, �������� ����� ����
		string fileContent; // ���������� �����

		AccessType accessType; // ��������� ����� � ������ ������.
		mutex *mut; // ��������� �� mutex, ��� ������ ����� �� ����� ���������������. ���� ���� ����� �������������� ������ ����� �������

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

	struct MyFS : IVFS
	{
		File root = File("root");

		string GetNextPartOfPath(string path) { // ��������������� �����, ������� ������ �� ���� ��� ���������� �����
			for (int i = 0; i < path.length(); i++) {
				if (path[i] == '\\')
				{
					return path.substr(0, i);
				}
			}
			return path;
		}

		bool FileNameIsValid(string namePart) {
			if (namePart.length() > 0) { //��� ����� ������ ��������� ���� �� 1 ������. ��� �������� �� ������ '\', ������ ��� �� ���������� ������� GetNextPartOfPath 
				return true;
			}
			return false;
		}

		File* Open(const char* name) { 
			// ������� ���� � readonly ������. ���� ��� ������ ����� ��� �� �� ������ �� writeonly ������ - ������� nullptr
			string path = string(name); // ���� ����� ������ "root\���� 1\���� 2\..\���� N\������� ����"
			File* currentFile; // ����, � ������� ��������� �������� � ������ ������
			string nextPart = GetNextPartOfPath(path); // �������� ���������� �����
			
			if (!nextPart._Equal("root")) { // ���� ���� ���������� �� � root, �� ���� ���������
				return nullptr;
			}
			else {
				currentFile = &root;
			}

			if (path.length() == nextPart.length() && root.accessType != WRITEONLY) { // ���� ���� ������� ������ �� root, �� ���������� ���
				currentFile->mut->lock();
				currentFile->accessType = READONLY;
				currentFile->mut->unlock();
				return currentFile;
			}
			else {
				path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));
			}
			
			map<string, File>::iterator it;

			while (true) {
				// ��������� �� ���� �������� ���������� �����
				nextPart = GetNextPartOfPath(path);
				if (path.length() != nextPart.length()) { 
					// ���� ��� ��� �������������� �����
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // �������� ����� ���� ����
					if (it == currentFile->children.end()) { 
						// ���� ��� �� ����������, �� ���������� nullptr
						return nullptr;
					}
					else { 
						// ���� �� ����������, ������� � ����
						currentFile = &(it->second);
					}
				}
				else { 
					// ���� ��� ��� ��������� �����
					it = currentFile->children.find(nextPart); // �������� ����� ���� ����
					if (it == currentFile->children.end()) { 
						// ���� ����� ��� � ���� ����������, ���������� nullptr
						return nullptr;
					}
					else if (it->second.accessType == WRITEONLY) {
						// ���� ���� � ������ writeonly, ���������� nullptr
						return nullptr;
					}
					else {
						// ���� ���� ���������� � �� ��������� �� � ������ writeonly
						File* resFile = &it->second;
						resFile->mut->lock();
						resFile->accessType = READONLY; // ��������� ���� � readonly
						resFile->mut->unlock();
						return resFile; // � ���������� ��������� �� ����
					}
				}
			}
		} 

		File* Create(const char* name){ 
			// ������� ��� ������� ���� � writeonly ������. ���� �����, �� ������� ��� ������ �������������, ���������� � ����. ������� nullptr, ���� ���� ���� ��� ������ � readonly ������.
			string path = string(name); // ���� ����� ������ "root\���� 1\���� 2\..\���� N\������� ����"
			File* currentFile; // ����, � ������� ��������� �������� � ������ ������
			string nextPart = GetNextPartOfPath(path); // �������� ���������� �����

			if (!nextPart._Equal("root")) { // ���� ���� ���������� �� � root, �� ���� ���������
				return nullptr;
			}
			else {
				currentFile = &root;
			}

			if (path.length() == nextPart.length()) { // ���� ���� ������� ������ �� root, �� ���������� ���
				currentFile->mut->lock();
				currentFile->accessType = WRITEONLY;
				currentFile->mut->unlock();
				return currentFile;
			}
			else {
				path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));
			}

			map<string, File>::iterator it;
			while (true) {
				// ��������� �� ���� �������� ���������� �����
				nextPart = GetNextPartOfPath(path);
				if (path.length() != nextPart.length()) {
					// ���� ��� ������������� ����
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // �������� ����� ���� ����
					if (it == currentFile->children.end()) {
						// ���� ��� �� ����������, �� ������ � ������� � ����.
						currentFile->children.insert(pair<string, File>(nextPart, File(nextPart)));
						currentFile = &currentFile->children.find(nextPart)->second;
					}
					else {
						// ���� �� ����������, ������� � ����
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
			if (f == nullptr) {
				return 0; // ���� ����� �� ����������, ���������� 0
			}
			f->mut->lock();
			if (f->accessType == READONLY) { // ���� ���� ������ ��� ������
				
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
				f->mut->unlock();
				return size(string(buff)); // ����������, ������� ���� ������� ���������
			}
			else { // ���� ���� ������ ��� writeonly
				f->mut->unlock();
				return 0; // ���������� 0 (������ �� ������� ���������)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
			if (f == nullptr) {
				return 0; // ���� ����� �� ����������, ���������� 0
			}
			f->mut->lock();
			if (f->accessType == WRITEONLY) { // ���� ���� ������ ��� ������
				// ���������� len �������� �� ������
				string bytesToWrite = string(buff).substr(0, len);
				f->fileContent = bytesToWrite;
				f->mut->unlock();
				return size(f->fileContent); // ����������, ������� ���� ������� �������� �� ������
			}
			else { // ���� ���� ������ ��� readonly
				f->mut->unlock();
				return 0; // ���������� 0 (������ �� ������� ��������)
			}
		}

		void Close(File* f) {
			// ������� ����	
			if (f == nullptr) {
				return; // ���� ����� �� ����������, ������� �� �������
			}
			f->mut->lock();
			f->accessType = CLOSED;
			f->mut->unlock();
		}
	};

}