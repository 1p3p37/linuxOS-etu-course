#include <iostream>
#include <sys/shm.h>
#include <unistd.h>

using namespace std;

void printArray(int *arr, const size_t size) {
    cout << "[";
    for(int i = 1; i < size; ++i) {
        cout << arr[i];
        if(i < size-1)
            cout << " ";
    }
    std::cout << "]\n";
}

int main(int arc, char* argv[]) {
    const size_t BUFFERSIZE = 11;
    const int firstKey = 1111;
    const int secondKey = 2222;
    int numOfIterations = atoi(argv[1]);

    int firstMemorySegment = shmget(firstKey, sizeof(int) * BUFFERSIZE, 0666 | IPC_CREAT);

    if(firstMemorySegment == -1) {
        cout << "Error in shmget()!" << endl;
        return -1;
    }

    int *sendArray = (int*)shmat(firstMemorySegment, nullptr, 0);

    if(sendArray == nullptr) {
        cout << "Error in shmat()!" << endl;
        return -1;
    }

    cout << "Launch second program to continue, waiting..." << endl;
    int secondMemorySegment;
    do {
        secondMemorySegment = shmget(secondKey, sizeof(int) * BUFFERSIZE, 0666);
    } while(secondMemorySegment == -1);

    int *receiveArray = (int*)shmat(secondMemorySegment, nullptr, 0);

    if(receiveArray == nullptr) {
        cout << "Error in shmat()!" << endl;
        return -1;
    }

    // заполняем массив случайными числами
    for(int i = 1; i < BUFFERSIZE; ++i) {
        sendArray[i] = rand() % 20;
    }

    while (numOfIterations--) {
        // отправка данных
        while(sendArray[0] == 1) {}
        cout << "sent:\t\t";
        printArray(sendArray, BUFFERSIZE);
        sendArray[0] = 1;

        // прием данных
        while(receiveArray[0] == 0) {}
        for(int i = 1; i < BUFFERSIZE; ++i)
            sendArray[i] = receiveArray[i];
        cout << "recieved:\t";
        printArray(receiveArray, BUFFERSIZE);
        receiveArray[0] = 0;
        cout << endl;
    }

    cout << "The work is done!" << endl;

    shmdt(sendArray);
    shmdt(receiveArray);
    shmctl(firstMemorySegment, IPC_RMID, 0);
}
