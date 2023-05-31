namespace TestTask
{
	struct File {
		// Вы имеете право как угодно задать содержимое этой структуры
	}; 

	struct IVFS
	{
		File *Open(const char* name) {
			// Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr

		} 

		File *Create(const char* name){ 
			// Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
		
		}

		size_t Read(File* f, char* buff, size_t len) {
			// Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
		
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
		
		}

		void Close(File* f) {
			// Закрыть файл	
		
		}
	};

}