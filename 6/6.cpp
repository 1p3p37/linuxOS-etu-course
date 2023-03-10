#include <iostream>
#include <chrono>
#include <signal.h>
#include <sys/time.h>
#include <wait.h>

using namespace std;

auto start = chrono::system_clock::now();
int repeat_num = 1;
int ri = 0;
bool stop_set = false;

void error(string);
void timer_act(int);

int main(int argc, char* argv[])
{
    sigset_t sset;
    struct sigaction act;
    struct timeval interval, value;
    struct itimerval timer;

    interval.tv_sec = atol(argv[1]);
    value.tv_sec = interval.tv_sec;
    repeat_num = atoi(argv[2]);

    timer.it_interval = interval;
    timer.it_value = value;

    sigemptyset(&sset); 
    sigaddset(&sset, SIGTSTP);
    sigprocmask(SIG_BLOCK, &sset, NULL); // блокируем сигнал от ctr-z
    act.sa_handler = timer_act;
    sigaction(SIGALRM, &act, NULL);

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) //запуск таймера
        error("Не удалось установить таймер");

    for (ri = 0; ri < repeat_num; ++ri)  // цикл ожидания
        pause();
    cout << "\nЗавершаюсь\n";
    return 0;
}

void error(string msg)
{
    perror(msg.c_str());
    exit(1);
}

void timer_act(int signal)
{
    pid_t pid;
    cout << endl << ri + 1 << ". Сработал таймер\n";

    auto child_start = chrono::system_clock::now();
    pid = fork();
    if (pid == 0)
    {
        time_t start_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        cout << "Потомок: идентификатор - " << getpid() << endl;
        cout << "Потомок: дата и время старта - " << ctime(&start_time);
        exit(0);
    }
    else if (pid == -1)
        error("Не удалось создать потомка");
    else
    {
        wait(NULL);
        auto end = chrono::system_clock::now();
        cout << "Время работы потомка: " << chrono::duration_cast<chrono::microseconds>(end - child_start).count() << " мкс\n";
        cout << "Время работы предка: " << chrono::duration<double>(end - start).count() << " с\n";
    }
}
