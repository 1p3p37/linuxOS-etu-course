#include <iostream>
#include <fstream>
#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

void writeFile(int, string, string);

int main()
{
    string filename;
    ofstream file;
    int delay0, delay1, delay2;

    setlocale(LC_ALL, "russian");

    cout << "Введите имя файла: ";
    cin >> filename;

    file.open(filename, ofstream::out | ofstream::app);
    if (!file.is_open())
    {
        cout << "\nНе удалось открыть файл (main)\n";
        return 0;
    }

    cout << "Введите времена задержек процессов\n";
    cout << "(Предок, потомок 1, потомок 2): ";
    scanf("%d %d %d", &delay0, &delay1, &delay2);
    cout << endl;

    file << "Время задержки процесса-предка: " << delay0 << " с" << endl;
    file << "Время задержки процесса-потомка 1: " << delay1 << " с" << endl;
    file << "Время задержки процесса-потомка 2: " << delay2 << " с" << "\n\n";
    file.close();
    
// создание потомков
    pid_t pid = fork();
    if (pid == 0)
        writeFile(delay1, filename, "Процесс-потомок 1 [fork()]");
    else if (pid > 0)
    {
        pid = vfork();
        if (pid == 0)
            execl("process2", filename.c_str(), to_string(delay2).c_str(), NULL);
        writeFile(delay0, filename, "Процесс-предок");
    }
    else
        return errno;

    return 0;
}

void writeFile(int delay, string filename, string name)
{
    ofstream file(filename, ofstream::out | ofstream::app);

    if (!file.is_open())
    {
        cout << "\nНе удалось открыть файл (writefile: " << name << ")\n";
        return;
    }

    sleep(delay);

    pid_t pid = getpid();
    file << name << ":\n\n";
    file << "Идентификатор процесса: " << pid << endl;
    file << "Идентификатор предка: " << getppid() << endl;
    file << "Идентификатор сессии: " << getsid(pid) << endl;
    file << "Идентификатор группы: " << getpgid(pid) << endl;
    file << "Реальный идентификатор пользователя: " << getuid() << endl;
    file << "Эффективный идентификатор пользователя: " << geteuid() << endl;
    file << "Реальный групповой идентификатор: " << getgid() << endl;
    file << "Эффективный групповой идентификатор: " << getegid() << "\n\n";
    file.close();

    cout << name << " завершил запись\n";
}

