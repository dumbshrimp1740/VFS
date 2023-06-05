#include <string>
#include <map>
#include <mutex>
#include <iostream>

using namespace std;

namespace TestTask
{
	enum AccessType { // Состояние файла
		READONLY, // Файл открыт в режиме чтения
		WRITEONLY, // Файл открыт в режиме записи
		CLOSED // Файл в данный момент свободен и его можно открыть
	};

	struct File {
		string fileName; // Имя файла
		map<string, File> children; // map, который хранит детей файла. Ключ — имя файла, значение — сам файл
		string fileContent; // Содержимое файла

		AccessType accessType; // Состояние файла в данный момент.
		mutex *mut; // Указатель на mutex, для защиты файла во время многопоточности. Один файл может использоваться только одним потоком

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
		virtual File* Open(const char* name) = 0; // Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
		virtual File* Create(const char* name) = 0; // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
		virtual size_t Read(File* f, char* buff, size_t len) = 0; // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
		virtual size_t Write(File* f, char* buff, size_t len) = 0; // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
		virtual void Close(File* f) = 0; // Закрыть файл	
	};

	struct MyFS : IVFS
	{
		File root = File("root");

		string GetNextPartOfPath(string path) { // Вспомогательный метод, который достаёт из пути имя следующего файла
			for (int i = 0; i < path.length(); i++) {
				if (path[i] == '\\')
				{
					return path.substr(0, i);
				}
			}
			return path;
		}

		bool FileNameIsValid(string namePart) {
			if (namePart.length() > 0) { //Имя файла должно содержать хотя бы 1 символ. Нет проверки на символ '\', потому что он отсекается методом GetNextPartOfPath 
				return true;
			}
			return false;
		}

		File* Open(const char* name) { 
			// Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
			string path = string(name); // Путь имеет формат "root\файл 1\файл 2\..\файл N\искомый файл"
			File* currentFile; // Файл, в котором находится алгоритм в данный момент
			string nextPart = GetNextPartOfPath(path); // Название следующего файла
			
			if (!nextPart._Equal("root")) { // Если путь начинается не с root, то путь невалиден
				return nullptr;
			}
			else {
				currentFile = &root;
			}

			if (path.length() == nextPart.length() && root.accessType != WRITEONLY) { // Если путь состоит только из root, то возвращаем его
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
				// Вычленяем из пути название следующего файла
				nextPart = GetNextPartOfPath(path);
				if (path.length() != nextPart.length()) { 
					// Если это имя промежуточного файла
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // Пытаемся найти этот файл
					if (it == currentFile->children.end()) { 
						// Если его не существует, то возвращаем nullptr
						return nullptr;
					}
					else { 
						// Если он существует, заходим в него
						currentFile = &(it->second);
					}
				}
				else { 
					// Если это имя конечного файла
					it = currentFile->children.find(nextPart); // Пытаемся найти этот файл
					if (it == currentFile->children.end()) { 
						// Если файла нет в этой директории, возвращаем nullptr
						return nullptr;
					}
					else if (it->second.accessType == WRITEONLY) {
						// Если файл в режиме writeonly, возвращаем nullptr
						return nullptr;
					}
					else {
						// Если файл существует и он находится не в режиме writeonly
						File* resFile = &it->second;
						resFile->mut->lock();
						resFile->accessType = READONLY; // Переводим файл в readonly
						resFile->mut->unlock();
						return resFile; // И возвращаем указатель на него
					}
				}
			}
		} 

		File* Create(const char* name){ 
			// Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
			string path = string(name); // Путь имеет формат "root\файл 1\файл 2\..\файл N\искомый файл"
			File* currentFile; // Файл, в котором находится алгоритм в данный момент
			string nextPart = GetNextPartOfPath(path); // Название следующего файла

			if (!nextPart._Equal("root")) { // Если путь начинается не с root, то путь невалиден
				return nullptr;
			}
			else {
				currentFile = &root;
			}

			if (path.length() == nextPart.length()) { // Если путь состоит только из root, то возвращаем его
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
				// Вычленяем из пути название следующего файла
				nextPart = GetNextPartOfPath(path);
				if (path.length() != nextPart.length()) {
					// Если это промежуточный файл
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // Пытаемся найти этот файл
					if (it == currentFile->children.end()) {
						// Если его не существует, то создаём и заходим в него.
						currentFile->children.insert(pair<string, File>(nextPart, File(nextPart)));
						currentFile = &currentFile->children.find(nextPart)->second;
					}
					else {
						// Если он существует, заходим в него
						currentFile = &(it->second);
					}
				}
				else {
					// Если это имя конечного файла
					it = currentFile->children.find(nextPart); // Пытаемся найти этот файл
					if (it != currentFile->children.end()) {
						if (it->second.accessType == READONLY) {
							// Если файл есть в этой директории и он readonly, возвращаем nullptr
							return nullptr;
						}
						else {
							// Если файл существует в этой директории и он не readonly, переводим его в writeonly
							it->second.mut->lock();
							it->second.accessType = WRITEONLY;
							it->second.mut->unlock();
							// Возвращаем указатель на файл
							return &it->second;
						}
					}
					else {
						// Если файла не существует, то создаём его и переводим в режим writeonly
						File resFile = File(nextPart);
						resFile.accessType = WRITEONLY;
						currentFile->children.insert(pair<string, File>(nextPart, resFile));
						return &currentFile->children.find(nextPart)->second;
					}
				}
			}
		}

		size_t Read(File* f, char* buff, size_t len) {
			// Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
			if (f == nullptr) {
				return 0; // Если файла не существует, возвращаем 0
			}
			f->mut->lock();
			if (f->accessType == READONLY) { // Если файл открыт для чтения
				
				// Если len больше длины буфера или если в него передали отрицательное число, то уменьшаем значение len до длины буфера.
				len = min(len, string(buff).length()); 

				// Читаем len символов и записываем их в buff, пока можем
				File tmpFile = *f;
				char* fileToRead = &tmpFile.fileContent[0];
				char* start = buff;
				for (size_t i = 0; i < len; i++) {
					char charToRead = *fileToRead;
					*buff = charToRead;
					buff++;
					fileToRead++;
				}
				if (len < string(buff).length()) { // Если длина буфера больше len, то лишние символы в конце буфера стираем
					for (size_t i = len; i < string(buff).length(); i++) {
						*buff = '\0';
						buff++;
					}
				}
				buff = start;
				f->mut->unlock();
				return size(string(buff)); // Возвращаем, сколько байт удалось прочитать
			}
			else { // Если файл закрыт или writeonly
				f->mut->unlock();
				return 0; // Возвращаем 0 (ничего не удалось прочитать)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
			if (f == nullptr) {
				return 0; // Если файла не существует, возвращаем 0
			}
			f->mut->lock();
			if (f->accessType == WRITEONLY) { // Если файл открыт для записи
				// Записываем len символов из буфера
				string bytesToWrite = string(buff).substr(0, len);
				f->fileContent = bytesToWrite;
				f->mut->unlock();
				return size(f->fileContent); // Возвращаем, сколько байт удалось записать из буфера
			}
			else { // Если файл закрыт или readonly
				f->mut->unlock();
				return 0; // Возвращаем 0 (ничего не удалось записать)
			}
		}

		void Close(File* f) {
			// Закрыть файл	
			if (f == nullptr) {
				return; // Если файла не существует, выходим из функции
			}
			f->mut->lock();
			f->accessType = CLOSED;
			f->mut->unlock();
		}
	};

}