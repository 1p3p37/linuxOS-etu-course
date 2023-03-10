#include <fstream>
#include <iostream>
#include <unistd.h>
#include <wait.h>

using namespace std;

void error(string);

int main()
{
    ifstream in_file("text.txt");
    string line;
    int fildes[2];
    int i = 1;
    sigset_t sset;
    pid_t pid1, pid2;

    if (!in_file.is_open())
        error("Не удалось открыть входной файл");
    cout << "Файл открыт\n";

    if (pipe(fildes) == -1)
        error("Не удалось открыть канал");
    cout << "Канал открыт\n";

    // Блокируем сигналы SIGUSR1, SIGUSR2 и SIGQUIT

    sigemptyset(&sset);
    sigaddset(&sset, SIGUSR1);
    sigaddset(&sset, SIGUSR2);
    sigaddset(&sset, SIGQUIT);
    sigprocmask(SIG_BLOCK, &sset, NULL);

    // Создаём потомков

    pid1 = vfork();
    if (pid1 == 0)
        execl("child", "child", "1", to_string(fildes[0]).c_str(), to_string(fildes[1]).c_str(), NULL);
    else if (pid1 == -1)
        error("Не удалось создать потомка 1");

    pid2 = vfork();
    if (pid2 == 0)
        execl("child", "child", "2", to_string(fildes[0]).c_str(), to_string(fildes[1]).c_str(), NULL);
    else if (pid2 == -1)
    {
        kill(pid1, SIGKILL);
        error("Не удалось создать потомка 2");
    }

    // Чтение файла

    cout << "Потомки созданы\n"
         << "Читаю файл\n";
    while(getline(in_file, line))
    {
        cout << i++ << ". " << "Получена строка. Записываю в канал\n";
        write(fildes[1], line.c_str(), line.size());
    }
    cout << "Чтение файла завершено. Ожидаю потомков\n";
    kill(0, SIGQUIT);
    while (wait(NULL) > 0);
    cout << "Потомки завершились\n"
         << "Завершаюсь\n";
    close(fildes[0]);
    close(fildes[1]);
    in_file.close();

    return 0;
}

void error(string msg)
{
    perror(msg.c_str());
    exit(1);
}
