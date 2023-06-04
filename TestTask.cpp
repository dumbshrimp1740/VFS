#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>
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
		map<string, File> children; // map, который хранит детей файла. Ключ — имя файла, значение — указатель на файл
		string fileContent; // Содержимое файла

		AccessType accessType; // Открыт или закрыт файл в данный момент.
		mutex *mut; // Указатель на mutex, для защиты файла во время многопоточности. По сути, один файл может использоваться только одним потоком

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

	struct MyVFS : IVFS
	{
		File root = File("root");

		string GetNextPartOfPath(string path) { // Вспомогательная функция для парсинга строк
			for (int i = 0; i < path.length(); i++) {
				if (path[i] == '\\')
				{
					return path.substr(0, i);
				}
			}
			return path;
		}

		File* Open(const char* name) { 
			// Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
			string path = string(name); // я исходил из того, что путь имеет формат "директория 1\директория 2\..\директория N\файл"
			File* currentFile = &root; // Директория, в которой находится алгоритм в данный момент

			string nextPart;
			map<string, File>::iterator it;

			while (true) {
				// Вычленяем из пути названия директорий и файла
				nextPart = GetNextPartOfPath(path);

				if (path.length() != nextPart.length()) { 
					// Если это имя директории
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // Пытаемся найти эту директорию
					if (it == currentFile->children.end()) { 
						// Если её не существует, то возвращаем nullptr
						return nullptr;
					}
					else { 
						// Если она существует, заходим в неё
						currentFile = &(it->second);
					}
				}
				else { 
					// Если это имя конечного файла
					it = currentFile->children.find(nextPart); // Пытаемся найти этот файл
					if (it == currentFile->children.end()) { 
						// Если файла нет в этой директории возвращаем nullptr
						cout << "there's no such file\n";
						return nullptr;
					}
					else if (it->second.accessType == WRITEONLY) {
						// Если файл в режиме writeonly, возвращаем nullptr
						cout << "File is WRITEONLY\n";
						return nullptr;
					}
					else {
						// Если файл существует и он находится в режиме readonly или закрыт 
						File* resFile = &it->second;
						resFile->mut->lock();
						resFile->accessType = READONLY; // То переводим файл в readonly
						resFile->mut->unlock();
						return resFile; // И возвращаем указатель на него
					}
				}
			}
		} 

		File* Create(const char* name){ 
			// Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
			string path = string(name); // я исходил из того, что путь имеет формат "директория 1\директория 2\..\директория N\файл"
			File* currentFile = &root; // Директория, в которой находится алгоритм в данный момент

			string nextPart;
			map<string, File>::iterator it;

			while (true) {
				// Вычленяем из пути названия директорий и файла
				nextPart = GetNextPartOfPath(path);

				if (path.length() != nextPart.length()) {
					// Если это имя директории
					path = path.substr((nextPart.length() + 1), (path.length() - nextPart.length() - 1));

					it = currentFile->children.find(nextPart); // Пытаемся найти эту директорию
					if (it == currentFile->children.end()) {
						// Если её не существует, то создаём её и заходим в неё.
						currentFile->children.insert(pair<string, File>(nextPart, File(nextPart)));
						currentFile = &currentFile->children.find(nextPart)->second;
					}
					else {
						// Если она существует, заходим в неё
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
			cout << "Read file " << f->fileName << endl;
			f->mut->lock();
			cout << "start Read file " << f->fileName << endl;
			if (f->accessType == READONLY) { //Если файл открыт для чтения
				
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
				cout << string(buff) << endl;
				f->mut->unlock();
				cout << "finish Read file " << f->fileName << endl;
				return size(string(buff)); // Возвращаем, сколько байт удалось прочитать
			}
			else { // Если файл закрыт или writeonly
				cout << "File is writeonly or closed\n";
				f->mut->unlock();
				cout << "finish Read file " << f->fileName << endl;
				return 0; // Возвращаем 0 (ничего не удалось прочитать)
			}
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
			cout << "Write file " << f->fileName << endl;
			f->mut->lock();
			this_thread::sleep_for(chrono::seconds(4));
			cout << "Start write file " << f->fileName << endl;
			if (f->accessType == WRITEONLY) { //Если файл открыт для записи
				// записываем len символов из буфера
				string bytesToWrite = string(buff).substr(0, len);
				f->fileContent = bytesToWrite;
				f->mut->unlock();
				cout << "Finish write file " << string(buff) << f->fileName << endl;
				return size(f->fileContent); // Возвращаем, сколько байт удалось записать из буфера
			}
			else { // Если файл закрыт или readonly
				f->mut->unlock();
				cout << "Finish write file " << f->fileName << endl;
				return 0; // Возвращаем 0 (ничего не удалось записать)
			}
		}

		void Close(File* f) {
			// Закрыть файл	
			cout << "Close file " << f->fileName << endl;
			f->mut->lock();
			cout << "start Close file " << f->fileName << endl;
			f->accessType = CLOSED;
			f->mut->unlock();
			cout << "Finish Close file " << f->fileName << endl;
		}
	};

}