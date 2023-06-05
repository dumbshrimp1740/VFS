# VFS

Тестовое задание от Петра Стольникова

## Немного о том, как я выполнял задание

Пока я решал задание, я поставил для себя следующие цели
1. Написать код, который работает;
2. Как можно сильнее его отдебажить;
3. Очень сильно его прокомментировать. Обычно я комментирую чуть меньше, но тут я действовал с идеей «Лучше я покажу весь ход своих мыслей, чтобы мой код лучше поняли и без живого общения со мной»;
4. Через коммиты показать, как я обычно коммичу на проектах;
5. Написать документацию. Немного дублирует пункт 3, но лучше излишне прокомментировать свой код, чем недокомментировать его.

## Структура файлов

* **TestTask.cpp** — моё решение.
* **VFS.cpp** — файл для тестов.

## File

**File** — структура, которая хранит в себе данные о файле. 

### Поля
**File** имеет 5 полей:
* **string fileName** — имя файла. Должен содержать хоть 1 символ и не должен содержать символ \\ (иначе алгоритмы не будут находить его, потому что этот символ отсекается при парсинге пути). _Можно задать через конструктор._
* **map<string, File> children** — map, который хранит детей файла. Ключ — имя файла, значение — сам файл. Изначально пустой
* **string fileContent** — содержимое файла. _Можно задать через конструктор._
* **AccessType accessType** — состояние файла в данный момент. Может быть в трёх состояниях:
  * **READONLY** — файл открыт только для чтения;
  * **WRITEONLY** — файл открыт только для записи;
  * **CLOSED** — файл закрыт. _Значение по умолчанию._
*  **mutex * mut** — указатель на mutex этого файла. Из-за него файлом одновременно может пользоваться только один поток. _Создаётся автоматически._

### Конструкторы

* **File(string name)** — создаёт файл с именем **name** и пустым содержанием.
* **File(string name, string content)** — создаёт файл с именем **name** и содержанием **content**.

## MyFS

**MyFS** — класс, который имплементирует интерфейс **IVFS**.
> **Важно!** Все пути в **MyFS** должны быть в формате **"root\файл 1\файл 2\..\файл N\искомый файл"**. Единственное исключение — путь "**root"**, который выдаёт корневой файл.  

> **Важно!** В **MyFS** всё является файлом.

### Поля

**MyFS** имеет всего одно поле:
* **File root** — корневой файл. _Создаётся автоматически_

### Конструкторы

* **MyFS()** — создаёт **MyFS** и файл **root** в ней

### Методы

#### string GetNextPartOfPath(string path)

Принимает путь или часть пути **(path)** и возвращает имя первого файла из него.
> **Пример 1** Если мы передадим этому методу путь _"file1\file2"_, то он вернёт нам имя _"file1"_
> **Пример 2** Если мы передадим этому методу путь _"file2"_, то он вернёт нам имя _"file2"_
> **Пример 3** Если мы передадим этому методу пустой путь, то он вернёт нам имя "". Это неправильный путь, но с этим разбираются другие методы.

#### bool FileNameIsValid(string namePart)

Принимает имя файла **(namePart)** и 
* Возвращает **true**, если имя файла непустое.
* Возвращает **false**, если имя файла пустое

Если требования к имени файла изменятся, их легко можно поменять здесь.
> **Имя файла не может содержать \\, почему здесь это не проверяется?**. Этот метод заточен под то, чтобы в него передавать результат работы **GetNextPartOfPath**, а этот метод по определению не может выдать строку с \\.

#### File* Open(const char* name)

Открывает файл в **READONLY** режиме. Если нет такого файла или же он открыт во **WRITEONLY** режиме — возвращает **nullptr**.
Также возвращает **nullptr**, если 
* Имя файла не соответствует правилам
* Путь не соответствует правилам
* Вместо **name** передали **nullptr**

#### File* Create(const char* name)

Открывает или создаёт файл в **WRITEONLY** режиме. Если нужно, создаёт все нужные поддиректории, упомянутые в пути. Если файл открыт в **READONLY** режиме — возвращает **nullptr**.
Также возвращает **nullptr**, если 
* Имя файла не соответствует правилам
* Путь не соответствует правилам
* Вместо **name** передали **nullptr**

#### size_t Read(File* f, char* buff, size_t len)

Читает **len** символов из файла **f** и записывает их в буфер **buff**.
Возвращает количество байт, которые реально удалось прочитать.

#### size_t Write(File* f, char* buff, size_t len)

Записывает **len** символов из буфера **buff** в файл **f**.
Возвращает количество байт, которые реально удалось записать.

#### void Close(File* f)

Переводит файл **f** в состояние **CLOSED**.

