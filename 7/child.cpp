#include <fstream>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;

string start_line = "Потомок ";
int fildes[2];
int sig_kill, sig_wait;
bool end_of_file = false;

void error(string);
void quit_action(int);

int main(int argc, char* argv[])
{
    int num = atoi(argv[1]);
    struct sigaction act_quit;
    ofstream out_file;
    string filename = "child_" + to_string(num);
    int signum;
    char ch;
    sigset_t set_blk, set_unblk, sset;

    // Инициализация

    act_quit.sa_handler = quit_action;
    sigemptyset(&act_quit.sa_mask);
    act_quit.sa_flags = 0;
    sigaction(SIGQUIT, &act_quit, NULL);

    start_line += to_string(num) + ": ";

    if (num == 1)
    {
        sig_kill = SIGUSR1;
        sig_wait = SIGUSR2;
    }
    else if (num == 2)
    {
        sig_kill = SIGUSR2;
        sig_wait = SIGUSR1;
    }
    else
    {
        cout << start_line << "Неверный аргумент\n";
        exit(1);
    }

    filename += ".txt";
    out_file.open(filename, ofstream::out | ofstream::trunc);
    if (!out_file.is_open())
        error("Не удалось открыть выходной файл");

    fildes[0] = atoi(argv[2]);
    fildes[1] = atoi(argv[3]);

    // Блокируем сигналы sig_kill и sig_wait
    sigemptyset(&set_blk);
    sigaddset(&set_blk, sig_kill);
    sigaddset(&set_blk, sig_wait);
    sigprocmask(SIG_BLOCK, &set_blk, NULL);

    cout << start_line << "Инициализирован\n";

    // Разблокируем сигнал SIGQUIT
    sigemptyset(&set_unblk);
    sigaddset(&set_unblk, SIGQUIT);
    sigprocmask(SIG_UNBLOCK, &set_unblk, NULL);

    if (num == 2)
        kill(0, SIGUSR2);

    // Чтение из канала

    while (!end_of_file)
    {
        sigemptyset(&sset);
        sigaddset(&sset, sig_wait);
        sigwait(&sset, &signum);

        if (signum != sig_wait)
            error("Неверный сигнал");

        if (read(fildes[0], &ch, 1) != -1)
        {
            out_file << ch;
            kill(0, sig_kill);
        }
        else
        {
            end_of_file = true;
            kill(0, sig_kill);
        }
    }

    close(fildes[0]);
    close(fildes[1]);
    out_file.close();

    return 0;
}

void error(string msg)
{
    string err_line = start_line + msg;
    perror(err_line.c_str());
    kill(0, sig_kill);
    exit(1);
}

void quit_action(int signal)
{
    cout << start_line << "Получен сигнал SIGQUIT\n";
    fcntl(fildes[0], F_SETFL, O_NONBLOCK);
}
