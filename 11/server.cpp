#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

using namespace std;
void bubble_sort(short * , short);
void timer_act(int);
void ctrl_z_act(int);
int listener, sock;
int main(int argc, const char * argv[]) {
  short * buf = nullptr;
  struct sigaction tact, zact;
  struct itimerval reset_timer;
  struct sockaddr_in addr_in;
  short len;
  pid_t child_pid;

  // Переопределяем реакцию на ctrl + z;
  sigemptyset( & zact.sa_mask);
  zact.sa_flags = 0;
  zact.sa_handler = ctrl_z_act;
  sigaction(SIGTSTP, & zact, NULL);

  // Заполнение структуры для таймера
  reset_timer.it_interval.tv_sec = 0;
  reset_timer.it_interval.tv_usec = 0;
  reset_timer.it_value.tv_usec = 0;
  reset_timer.it_value.tv_sec = atol(argv[2]); // Ждать заданное кол-во секунд

  // Переопределяем реакцию на таймер
  sigemptyset( & tact.sa_mask);
  tact.sa_flags = 0;
  tact.sa_handler = timer_act;
  sigaction(SIGALRM, & tact, NULL);

  // Открытие сокета
  listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    perror("Не удалось открыть сокет");
    exit(1);
  }

  // Заполнение структуры адреса
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(3434);
  addr_in.sin_addr.s_addr = htonl(INADDR_ANY);

  // Именование сокета
  if (bind(listener, (struct sockaddr * ) & addr_in, sizeof(addr_in)) < 0) {
    perror("Не удалось именовать сокет");
    close(listener);
    exit(2);
  }

  // Запуск очереди соединений и таймера
  cout << "Ожидаю клиентов\n";
  listen(listener, atoi(argv[1])); // Очередь заданной длины
  setitimer(ITIMER_REAL, & reset_timer, NULL);

  // Цикл ожидания и обработки запросов
  while (1) {
    sock = accept(listener, NULL, NULL);
    if (sock < 0) {
      if (errno != EINTR) {
        perror("Не удалось принять запрос");
        close(listener);
        exit(3);
      }
    }
    cout << "\nПришёл запрос. Создаю потомка\n";
    child_pid = fork();
    if (child_pid == 0) // Процесс потомка
    {
      if (recv(sock, & len, sizeof(short), 0) < 0) // Принимаем
      {
        perror("Потомок: ошибка при приёме размера массива");
        close(sock);
        exit(1);
      }
      cout << "Потомок: принял размер массива - " << len << endl;


      // Выделение памяти
      buf = (short * ) malloc(len * sizeof(short));
      if (recv(sock, buf, len * sizeof(short), 0) < 0) {
        perror("Потомок: ошибка при приёме массива");
        close(sock);
        exit(2);
      }
      cout << "Потомок: принял массив. Сортирую\n";
      bubble_sort(buf, len); // Сортируем
      cout << "Потомок: сортировка завершена. Отправляю клиенту\n";
      if (send(sock, buf, len * sizeof(short), 0) < 0) // Отправляем
      {
        perror("Потомок: не удалось отправить ответ");
      }
      close(sock); // Закрываем
      cout << "Потомок: завершаюсь\n";
      exit(0);
    } else if (child_pid < 0) {
      perror("Не удалось создать потомка");
      close(sock);
      close(listener);
      exit(4);
    }
    setitimer(ITIMER_REAL, & reset_timer, NULL); // Перезапуск таймера
  }
  close(listener);
  return 0;
}

void bubble_sort(short * arr, short n) {
  int i, j;
  short num;
  for (i = 0; i < n - 1; ++i)
    for (j = 0; j < n - i - 1; ++j)
      if (arr[j] > arr[j + 1]) {
        num = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = num;
      }
}

void timer_act(int signal) {
  cout << "\nПревышено время ожидания клиентов\n";
  close(listener);
  exit(0);
}

void ctrl_z_act(int signal) {
  cout << "\nОстановка программы\n";
  close(sock);
  close(listener);
  exit(-1);
}