#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

// Структура "аргументы потока"
struct thread_args
{
    int &file;
    const string &str;
};

// Функция потоков
void* threadFunc(void*);

// Функция порождения потоков
void initThread(pthread_t*, thread_args*);

int main()
{
    pthread_t thread1, thread2;
    string line, line2;
    ifstream inFile;
    int outFile1, outFile2;

    // Открытие файлов

    inFile.open("text.txt");
    if (!inFile.is_open())
    {
        cout << "Не удалось открыть файл\n";
        exit(1);
    }

    if ((outFile1 = creat("/home/pepe/LinuxLabs/4/output_file_1.txt", S_IRUSR | S_IWUSR)) < 0)
    {
        perror("outFile1 creat()");
        exit(1);
    }

    if ((outFile2 = creat("/home/pepe/LinuxLabs/4/output_file_2.txt", S_IRUSR | S_IWUSR)) < 0)
    {
        perror("outFile2 creat()");
        exit(1);
    }

    // Чтение файла

    cout << "Читаю файл\n";
    int i = 1;
    while (getline(inFile, line))
    {
        line += '\n';
        cout << i++ << ". Получена строка. Создаю поток 1\n";
        thread_args args{outFile1, line};
        initThread(&thread1, &args);

        if (getline(inFile, line2))
        {
            line2 += '\n';
            cout << i++ << ". Получена строка. Cоздаю поток 2\n";
            thread_args args2{outFile2, line2};
            initThread(&thread2, &args2);
            pthread_join(thread2, NULL);
        }
        pthread_join(thread1, NULL);
    }

    // Закрытие файлов

    inFile.close();
    close(outFile1);
    close(outFile2);

    return 0;
}

// Функция потоков

void* threadFunc(void* a)
{
    thread_args *args = (thread_args*) a;

    write(args->file, args->str.c_str(), args->str.length());

    return nullptr;
}

// Функция порождения потоков

void initThread(pthread_t* thread, thread_args *args)
{
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    if (pthread_create(thread, &attrs, threadFunc, (void*) args) != EXIT_SUCCESS)
    {
        perror("initThread: pthread_create()");
        exit(1);
    }
}
