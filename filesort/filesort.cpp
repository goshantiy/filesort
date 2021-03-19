// files_sort.cpp : Defines the entry point for the console application.
//
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>

// генерация N случайных чисел в диапозоне от max до min в file
void randomFile(std::string& file, int n, int max = 500, int min = 0)
{
	srand(time(0));
	std::ofstream out(file);
	for (int i = 0; i < n; i++)
	{
		int x = (double)rand() / RAND_MAX*(max - min) + min;
		out << x << ' ';
	}
	out.close();
}

// проверка корректности сортировки
bool checkSort(std::string& file)
{
	std::ifstream in(file);
	int x, y;
	in >> x;
	if (!in.eof())
	{
		in >> y;
		while (!in.eof())
		{
			if (x > y)
			{
				return false;
				in.close();
			}
			x = y;
			in >> y;
		}
	}
	in.close();
	return true;
}

// многофазная сортировка с заданной степенью разбиения
void mergeSort(std::string& file, int n)
{
	// минимальная степень 3, 2 для разбиения, 1 для слияния
	if (n < 3)
		n = 3;

	// answer файл начальных и конечных значений
	std::fstream answer(file, std::ios::in);

	// проверяем наличие каких-либо данных в файле
	int x;
	answer >> x;
	if (!answer.eof())
	{
		answer.close();
		answer.open(file, std::ios::in);
		std::fstream** files = new std::fstream * [n];// массив вспомогательных файлов
		char** files_names = new char* [n];// названия вспомогательных файлов, для открытия\закрытия файлов после сдвига указателей
		x = 2 + log10(n);// кол-во символов, необходимое для записи названий файлов 
		for (int i = 0; i < n; i++)
		{
			int b;
			files_names[i] = new char[x];			
			b = sprintf_s(files_names[i], x, "%d", i);//sprintf аналог printf, записывает результат в массив, возвращает число записанных символов 
			files_names[i][b] = '\0';
			files[i] = new std::fstream(files_names[i], std::ios::out);
		}

		// количество уровней разбиения
		int level = 1;
		// массив частичных сумм, необходим для рассчета фиктивных отрезков в файлах
		int* a = new int[n];
		//на этапе разбиения: число отрезков, необходимое для перехода от одного идеального распределения к другому
		//на этапе слияния: число фиктивных отрезков в каждом рабочем файле
		int* d = new int[n];
		// начальные значения массивов
		for (int i = 0; i < n - 1; i++)
			d[i] = a[i] = 1;
		d[n - 1] = a[n - 1] = 0;

		// разбиение
		
		int k = 0;// K - номер файла куда будет записан отрезок
		answer >> x;
		while (!answer.eof())//пока не конец файла
		{
			int y = 0;
			// записываем упорядоченный отрезок
			for (; !answer.eof() && x >= y;)
			{
				*files[k] << ' ' << x;
				y = x;
				answer >> x;
			}
			*files[k] << " -1";//ограничиваем отрезки
			d[k]--;

			// учет горизонтального распределения фиктивных отрезков
			if (!answer.eof())
			{
				if (d[k] < d[k + 1])
					k++;
				else
					if (d[k] == 0)
					{
						level++;
						k = 0;
						int tmp = a[0];
						for (int i = 0; i < n - 1; i++)
						{
							d[i] = a[i + 1] - a[i] + tmp;
							a[i] = a[i + 1] + tmp;
						}
						

					}
					else
						k = 0;
			}
		}

		answer.close();
		// слияние
		// открываем все, кроме последнего файла, на чтение
		for (int i = 0; i < n - 1; i++)
		{
			files[i]->close();
			files[i]->open(files_names[i], std::ios::in);
		}

		while (level)//если level=0 сортировка завершена
		{
			// наличие фиктивных отрезков во всех файлах
			bool haveFict = true;

			while (!files[n - 2]->eof())
			{
				// удаление фиктивных отрезков 
				for (int i = 0; i < n - 1; i++)
				{
					if (d[i])
					{
						d[i]--;
						a[i] = -1;
					}
					else
					{
						// в файле закончились фиктивные отрезки, записываем первый элемент из реального отрезка в i-тую ячейку массива
						haveFict = false;
						*files[i] >> a[i];
					}
				}

				if (haveFict)
					d[n - 1]++;
				else//слияние
				{
					// проверка на наличие реальных отрезков для слияния
					bool check;

					do
					{
						check = false;
						int min = INT32_MAX, ix;
						//как в многопутевом
						//ищем минимальный элемент, номер файла и проверяем, что отрезки не закончились
						for (int i = 0; i < n - 1; i++)
							if (a[i] != -1 && a[i] < min)
							{
								check = true;
								min = a[i];
								ix = i;
							}

						// записываем минимальный элемент и берем новый элемент из файла
						if (check)
						{
							*files[n - 1] << ' ' << min;
							*files[ix] >> a[ix];
						}

					} while (check);
					*files[n - 1] << " -1";
				}
			}

			files[n - 2]->close();
			files[n - 1]->close();

			files[n - 2]->open(files_names[n - 2], std::ios::out);
			files[n - 1]->open(files_names[n - 1], std::ios::in);

			// сдвиг указателей на файлы и их имена
			int D = d[n - 1];
			char* fn = files_names[n - 1];
			std::fstream* fl = files[n - 1];

			for (int i = n - 1; i > 0; i--)
			{
				d[i] = d[i - 1];
				files_names[i] = files_names[i - 1];
				files[i] = files[i - 1];
			}
			d[0] = D;
			files_names[0] = fn;
			files[0] = fl;

			level--;
		}


		// записываем ответ в исходный файл
		answer.open(file, std::ios::out);

		for (*files[0] >> x; x != -1; *files[0] >> x)
			answer << x << ' ';


		// закрываем, все что открыли, и удаляем вспомогательные файлы
		for (int i = 0; i < n; i++)
		{
			files[i]->close();
			delete files[i];
			std::remove(files_names[i]);
			delete[] files_names[i];
		}
		delete[] files;
		delete[] files_names;
		delete[] a;
		delete[] d;
	}
	answer.close();
}


int main()
{
	std::string file = "file.txt";

	randomFile(file, 500000);

	mergeSort(file, 3);

	std::fstream out(file, std::ios::app);
	if (checkSort(file))
		out << "\nSorted";
	else
		out << "\nUnsorted";

	out.close();
	return 0;
}

