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
    cout << "]\n";
}

int main(int arc, char* argv[]) {
    const size_t BUFFERSIZE = 11;
    const int firstKey = 1111;
    const int secondKey = 2222;
    int numOfIterations = atoi(argv[1]);

    int secondMemorySegment = shmget(secondKey, sizeof(int)*BUFFERSIZE, 0666 | IPC_CREAT);

    if(secondMemorySegment == -1){
        cout << "Error in shmget()!" << endl;
        return -1;
    }

    int *sendArray = (int*)shmat(secondMemorySegment, nullptr, 0);

    if(sendArray == nullptr) {
        cout << "Error in shmat()!" << endl;
        return -1;
    }

    cout << "Launch first program to continue, waiting..." << endl;
    int firstMemorySegment;
    do {
        firstMemorySegment = shmget(firstKey, sizeof(int)*BUFFERSIZE, 0666);
    } while(firstMemorySegment == -1);

    int *receiveArray = (int*)shmat(firstMemorySegment, nullptr, 0);

    if(receiveArray == nullptr) {
        cout << "Error in shmat()!" << endl;
        return -1;
    }

    while (numOfIterations--) {
        // прием данных
        while (receiveArray[0] == 0) {}
        for(int i = 1; i < BUFFERSIZE; ++i)
            sendArray[i] = receiveArray[i]-1;
        cout << "recieved\t";
        printArray(receiveArray, BUFFERSIZE);
        receiveArray[0] = 0;

        // отправка данных
        while(sendArray[0] == 1) {}
        cout << "sent:\t\t";
        printArray(sendArray, BUFFERSIZE);
        sendArray[0] = 1;
        cout << endl;
    }

    cout << "The work is done!" << endl;

    shmdt(sendArray);
    shmdt(receiveArray);
    shmctl(secondMemorySegment, IPC_RMID, 0);
}
