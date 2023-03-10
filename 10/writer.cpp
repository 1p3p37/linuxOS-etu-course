#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/sem.h>

using namespace std;
int main(int argc,
  const char * argv[]) {
  struct sembuf op1 = {0,1,0}, // Добавление писателя
    op2 = {0,-1,0}, // Удаление писателя
    op3 = {1,0,0}, // Ожидание читателей
    op4 = {2,1,0}, // Семафор, который отвечает за добавление рабочих процессов;Данные семафоры отвечают за завершения работы программы
    op5 = {2,1,0}, // Семафор, который отвечает за уменьшение кол-ва рабочих процессов
    op6 = {3,1,0}, // Выход из критической зоны писателя
    op7 = {3,-1,0}; // Вход в критическую зону писателя
  ofstream out_file;
  int sem_key, sleep_time, str_num, process_num, sem_id;
  sem_key = atoi(argv[1]); // Ключ семафора
  sleep_time = atoi(argv[2]); // Задержка
  str_num = atoi(argv[3]); // Количество строк, которые записываются в файл писателем
  process_num = atoi(argv[4]); // Номер писателя
  if ((sem_id = semget(sem_key, 4, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
    if (errno == EEXIST) {
      if ((sem_id = semget(sem_key, 4, IPC_CREAT)) == -1) {
        perror("Не удалось подключить множественный семафор");
        exit(1);
      } else {
        cout << "Множественный семафор подключен\n";
        if (semctl(sem_id, 0, GETVAL) == 0)
          semctl(sem_id, 3, SETVAL, 1);
      }
    } else {
      perror("Не удалось создать множественный семафор");
      exit(1);
    }
  } else {
    cout << "Множественный семафор создан\n";
    // Присваиваем начальные значения семафорам
    semctl(sem_id, 0, SETVAL, 0);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 0);
    semctl(sem_id, 3, SETVAL, 1);
  }
  semop(sem_id, & op4, 1); // Увеличиваем количество рабочих процессов

  for (int i = 0; i < str_num; ++i) {
    semop(sem_id, & op1, 1); // Добавление писателя
    semop(sem_id, & op3, 1); // Ожидание окончание работы читателей
    semop(sem_id, & op7, 1); // Вход в критическую зону
    out_file.open("File.txt", ios::app);
    out_file << "Писатель с номером процесса " << process_num << endl;
    cout << "Писатель с номером процесса " << process_num << endl;
    out_file.close();
    semop(sem_id, & op6, 1); // Выход из критической зоны
    semop(sem_id, & op2, 1); // Удаление писателя
    sleep(sleep_time); // Ожидание
  }
  semop(sem_id, & op5, 1); // Уменьшение кол-ва процессов
  if (semctl(sem_id, 2, GETVAL) == 0) // Проверка: если кол-во значение семафора 3 = 0, значит все рабочие процессы завершены и можно завершать работу программы
    semctl(sem_id, 0, IPC_RMID, 0); // Удаление семафора писателя
  return 0;
}