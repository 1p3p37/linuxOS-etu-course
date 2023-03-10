#include <iostream>
#include <queue>
#include <sys/msg.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

struct Request
{
    int qid;
    time_t rtime;
};

struct msg_request
{
    long pnum;
    struct Request msg;
};

struct msg_answer
{
    long pnum;
    time_t atime;
};

int qid, myqid, fd;

void error(const char*);

int main(int argc, const char* argv[])
{
    const int MAXSIZE = 1024;

    short answers = 0;
    short answered = 0;
    bool have_read_file = false;
    bool active = true;

    char buf[MAXSIZE];
    struct msg_request req;
    struct msg_answer ans;
    struct msqid_ds stat;
    int qkey;
    short this_prog_num, prog2_num, prog3_num;
    time_t req_time;
    long endtask;
    queue<msg_request> requests;

    // Инициализация

    if (argc < 2)
    {
        // Создание общей очереди сообщений
        qkey = 0;
        while ((qid = msgget(++qkey, IPC_CREAT | IPC_EXCL | 0622)) == -1)
        {
            if (errno != EEXIST)
            {
                perror("Не удалось создать общую очередь сообщений");
                exit(1);
            }
        }
        cout << "Создал общую очередь сообщений\n";
        cout << "Ключ очереди: " << qkey << endl;

        // Определение номеров программм
        this_prog_num = 1;
        prog2_num = 2;
        prog3_num = 3;
    }
    else if (argc != 3)
    {
        cout << "Неправильное количество аргументов\n";
        exit(1);
    }
    else
    {
        // Определение номеров программ
        prog2_num = 1;
        this_prog_num = atoi(argv[2]);
        if (this_prog_num == 2)
            prog3_num = 3;
        else if (this_prog_num == 3)
            prog3_num = 2;
        else
        {
            cout << "Неправильный аргумент 2\n";
            exit(1);
        }

        // Получение идентификатора общей очереди сообщений
        qkey = atoi(argv[1]);
        if ((qid = msgget(qkey, 0)) == -1)
        {
            perror("Не удалось открыть общую очередь сообщений");
            exit(1);
        }
        cout << "Открыл общую очередь сообщений\n";
    }

    // Создание собственной очереди сообщений
    if ((myqid = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0622)) == -1)
    {
        perror("Не удалось создать свою очередь сообщений");
        if (msgctl(qid, IPC_RMID, nullptr) == -1)
            perror("Не удалось закрыть общую очередь сообщений");
        exit(1);
    }
    cout << "Создал свою очередь сообщений\n";

    // Открытие файла
    if ((fd = open("/home/pepe/LinuxLabs/8/text.txt", 0)) == -1)
        error("Не удалось открыть файл");


    // Начало работы

    cout << "Посылаю запросы на чтение\n";
    req_time = time(nullptr);

    req.pnum = prog2_num;
    req.msg.qid = myqid;
    req.msg.rtime = req_time;
    if (msgsnd(qid, &req, sizeof(struct Request), 0) == -1)
        error("Не удалось отправить запрос 1");

    req.pnum = prog3_num;
    if (msgsnd(qid, &req, sizeof(struct Request), 0) == -1)
        error("Не удалось отправить запрос 2");

    cout << "Программа " << this_prog_num << endl;
    while (active)
    {
        if (msgrcv(qid, &req, sizeof(struct Request), this_prog_num, IPC_NOWAIT) != -1)
        {
            if (req.msg.rtime < req_time)
            {
                ans.pnum = this_prog_num;
                ans.atime = time(nullptr);
                if (msgsnd(req.msg.qid, &ans, sizeof(time_t), 0) == -1)
                    error("Не удалось отправить ответ");
                ++answered;
            }
            else
                requests.push(req);
        }
        else if (errno != ENOMSG)
            error("Не удалось проверить общую очередь сообщений");

        if (!have_read_file)
        {
            if (msgrcv(myqid, &ans, sizeof(time_t), 0, IPC_NOWAIT) != -1)
            {
                ++answers;
                cout << "Время ответа программы " << ans.pnum << ": " << asctime(localtime(&ans.atime));
                if (answers == 2)
                {
                    if (read(fd, (void*)buf, MAXSIZE) != -1)
                    {
                        cout << "Содержимое файла:\n" << buf << endl;
                        have_read_file = true;
                        close(fd);
                    }
                    else
                        error("Не удалось прочитать файл");
                }
            }
            else if (errno != ENOMSG)
                error("Не удалось проверить свою очередь сообщений");
        }
        else if (!requests.empty())
        {
            req = requests.front();
            requests.pop();
            ans.pnum = this_prog_num;
            ans.atime = time(nullptr);
            if (msgsnd(req.msg.qid, &ans, sizeof(time_t), 0) == -1)
                error("Не удалось отправить ответ");
            ++answered;
        }

        if ((answered == 2) && have_read_file)
            active = false;
    }

    if (msgctl(myqid, IPC_RMID, nullptr) == -1)
        error("Не удалось закрыть свою очередь сообщений");
    cout << "Закрыл свою очередь сообщений\n";

    if (msgctl(qid, IPC_STAT, &stat) == -1)
        error("Не удалось просмотреть статистику общей очереди");

    if (stat.msg_lspid == getpid())
    {
        if (msgctl(qid, IPC_RMID, nullptr) == -1)
            error("Не удалось закрыть общую очередь сообщений");
        cout << "Закрыл общую очередь сообщений\n";
    }
    cout << "Завершаюсь\n";

    return 0;
}

void error(const char* msg)
{
    perror(msg);

    if (msgctl(myqid, IPC_RMID, nullptr) == -1)
        perror("Не удалось закрыть свою очередь сообщений");

    if (msgctl(qid, IPC_RMID, nullptr) == -1)
        perror("Не удалось закрыть общую очередь сообщений");
    close(fd);

    exit(1);
}
