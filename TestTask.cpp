namespace TestTask
{
	struct File {
		// �� ������ ����� ��� ������ ������ ���������� ���� ���������
	}; 

	struct IVFS
	{
		File *Open(const char* name) {
			// ������� ���� � readonly ������. ���� ��� ������ ����� ��� �� �� ������ �� writeonly ������ - ������� nullptr

		} 

		File *Create(const char* name){ 
			// ������� ��� ������� ���� � writeonly ������. ���� �����, �� ������� ��� ������ �������������, ���������� � ����. ������� nullptr, ���� ���� ���� ��� ������ � readonly ������.
		
		}

		size_t Read(File* f, char* buff, size_t len) {
			// ��������� ������ �� �����. ������������ �������� - ������� ������� ���� ������� ���������
		
		} 

		size_t Write(File* f, char* buff, size_t len) {
			// �������� ������ � ����. ������������ �������� - ������� ������� ���� ������� ��������
		
		}

		void Close(File* f) {
			// ������� ����	
		
		}
	};

}