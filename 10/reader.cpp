#include <iostream>

#include <fstream>

#include <sys/sem.h>

using namespace std;

int main(int argc, const char * argv[]) {

  struct sembuf op1 = {0,0,0 }, // Проверка наличия писателей
  op2 = { 1, 1, 0 }, // Добавление читателя
  op3 = { 1, -1, 0 }, // Удаление читателя
  op4 = { 2, 1, 0 }, // Семафор, который отвечает за добавление рабочих процессов;Данный семафор отвечает за завершения работы программы
  op5 = { 2, -1, 0 }; // Семафор, который отвечает за уменьшение кол-ва рабочих процессов

  ifstream in_file;
  string str;

  int sem_key, sem_id, sleep_time, process_num;
  sem_key = atoi(argv[1]);
  process_num = atoi(argv[2]);

  if ((sem_id = semget(sem_key, 4, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
    if (errno == EEXIST) {
      if ((sem_id = semget(sem_key, 4, IPC_CREAT)) == -1) {
        perror("Не удалось подключить множественный семафор");
        exit(1);
      } else
        cout << "Множественный семафор подключен\n";
    } else {
      perror("Не удалось создать множественный семафор");
      exit(1);
    }
  } else {
    cout << "Множественный семафор создан\n";
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 0);
  }

  semop(sem_id, & op4, 1); // Увеличиваем кол-во процессов
  semop(sem_id, & op1, 1); // Проверяем нет ли писателей
  semop(sem_id, & op2, 1); // Добавление читателя
  in_file.open("File.txt");

  while (getline(in_file, str) && !in_file.eof())
    cout << "Читатель " << process_num << " вывел строку: " << str << endl;
  in_file.close();

  semop(sem_id, & op3, 1); // Удаление читателя
  semop(sem_id, & op5, 1); // Уменьшение кол-ва процессов
  if (semctl(sem_id, 2, GETVAL) == 0)
    semctl(sem_id, 0, IPC_RMID, 0); // Удаление семафора читателя
  return 0;
}


