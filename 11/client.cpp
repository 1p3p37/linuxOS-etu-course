#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

using namespace std;
void timer_act(int);
void ctrl_z_act(int);
int sock;
int main(int argc, const char * argv[]) {
  bool repeating = true;
  short * buf;
  struct sigaction tact, zact;
  struct sockaddr_in addr_in;
  struct timeval timeout;
  struct itimerval timerval;
  short len;
  fd_set fdset;
  // Переопределяем реакцию на ctrl + z;
  sigemptyset( & zact.sa_mask);
  zact.sa_flags = 0;
  zact.sa_handler = ctrl_z_act;
  sigaction(SIGTSTP, & zact, NULL);
  // Переопределение реакции на таймер
  sigemptyset( & tact.sa_mask);
  tact.sa_flags = 0;
  tact.sa_handler = timer_act;
  sigaction(SIGALRM, & tact, NULL);
  // Заполнение структуры для таймера
  timeout.tv_usec = 0;
  timeout.tv_sec = atol(argv[1]); // Установка времени ожидания
  timerval.it_interval.tv_usec = 0;
  timerval.it_interval.tv_sec = 0;
  timerval.it_value = timeout;
  // Открытие сокета
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("не удалось открыть сокет");
    exit(1);
  }
  // Заполнение структуры адреса
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(3434);
  addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
  // Установка соединения
  cout << "Ожидаю сервер\n";
  setitimer(ITIMER_REAL, & timerval, NULL); // Установка таймера
  timerval.it_value.tv_sec = 0;

  while (repeating) {
    if (connect(sock, (struct sockaddr * ) & addr_in, sizeof(addr_in)) < 0) {
      if (errno != ECONNREFUSED) {
        perror("Не удалось подключиться к серверу");
        close(sock);
        exit(2);
      }
    } else
      repeating = false;
  }

  setitimer(ITIMER_REAL, & timerval, NULL);
  cout << "Подключён\n";

  // Генерация случайных чисел
  srand(time(NULL));
  len = 10 + (rand() % 11); // Длина массива от 10 до 20
  buf = (short * ) malloc(len * sizeof(short));
  cout << "Сгенерированная последовательность числел: ";
  for (int i = 0; i < len; ++i) {
    buf[i] = rand() % 10; // Случайное число от 0 до 9
    cout << buf[i] << ' ';
  }
  cout << endl;
  cout << "Отправляю размер массива\n";
  if (send(sock, & len, sizeof(short), 0) < 0) {
    perror("не удалось отправить размер массива");
    close(sock);
    exit(3);
  }
  cout << "Отправляю массив\n";
  if (send(sock, buf, len * sizeof(short), 0) < 0) {
    perror("Не удалось отправить массив");
    close(sock);
    exit(4);
  }

  // Ожидание ответа
  FD_ZERO( & fdset); // Очищаем сет
  FD_SET(sock, & fdset); // Добавляем сокет в сет
  cout << "Ожидаю ответа\n";
  int sel = select(sock + 1, & fdset, NULL, NULL, & timeout);
  if (sel < 0) // Ожидаем смены состояния сокета на возможность чтения данных
  {
    perror("select");
    close(sock);
    exit(5);
  } else if (sel == 0) {
    cout << "Превышено время ожидания\n";
    shutdown(sock, 2);
    close(sock);
    exit(0);
  }

  if (recv(sock, buf, len * sizeof(short), 0) < 0) {
    perror("Не удалось принять массив");
    close(sock);
    exit(6);
  }
  cout << "Принял упорядоченную последовательность: ";
  for (int i = 0; i < len; ++i)
    cout << buf[i] << ' ';
  cout << endl << "Завершаю работу\n";
  close(sock);
  return 0;
}

void timer_act(int signal) {
  cout << "Превышено время ожидания\n";
  close(sock);
  exit(0);
}

void ctrl_z_act(int signal) {
  cout << "\nОстановка программы\n";
  shutdown(sock, 2);
  close(sock);
  exit(-1);
}