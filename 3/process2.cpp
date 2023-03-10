#include <iostream>
#include <fstream>
#include <locale.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[])
{
    ofstream file(argv[0], ofstream::out | ofstream::app);

    if (!file.is_open())
    {
        cout << "\nНе удалось открыть файл (Процесс-потомок 2)\n";
        return 0;
    }

    sleep(atoi(argv[1]));

    pid_t pid = getpid();
    file << "Процесс-потомок 2 [vfork()]:\n\n";
    file << "Идентификатор процесса: " << pid << endl;
    file << "Идентификатор предка: " << getppid() << endl;
    file << "Идентификатор сессии: " << getsid(pid) << endl;
    file << "Идентификатор группы: " << getpgid(pid) << endl;
    file << "Реальный идентификатор пользователя: " << getuid() << endl;
    file << "Эффективный идентификатор пользователя: " << geteuid() << endl;
    file << "Реальный групповой идентификатор: " << getgid() << endl;
    file << "Эффективный групповой идентификатор: " << getegid() << "\n\n";
    file.close();

    cout << "Процесс-потомок 2 [vfork()] завершил запись\n";

    return 0;
}

