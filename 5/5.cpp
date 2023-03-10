#include <iostream>
#include <signal.h>

using namespace std;

void error_handler(int);

int main()
{
    struct sigaction action;
    short error_input;
    bool correct_input;

    action.sa_handler = error_handler;

    do
    {
        correct_input = true;

        cout << "1: Деление на ноль\n"
             << "2: Нарушение защиты памяти\n"
             << "0: Выход\n"
             << "\n> ";
        cin >> error_input;

        switch (error_input)
        {
        case 1:
            {
                sigaction(SIGFPE, &action, NULL);
                cout << "\n1 / 0 =" << 1 / 0;
            }
        case 2:
            {
                sigaction(SIGSEGV, &action, NULL);
                cout << "Запись значения 0 в arr[0]";
                int* arr = nullptr;
                arr[0] = 0;
            }
        case 0: break;

        default:
            {
                correct_input = false;

                cout << "Неверный ввод";
                getchar();
                while (getchar() != '\n');

                system("clear");
            }
        }
    }
    while(!correct_input);

    return 0;
}

void error_handler(int error_type)
{
    int exit_code = 0;

    switch (error_type)
    {
    case SIGFPE:
        {
            cout << "\nДеление на ноль!\n";
            exit_code = 1;
            break;
        }

    case SIGSEGV:
        {
            cout << "\nНарушение защиты памяти!\n";
            exit_code = 2;
            break;
        }
    }

    exit(exit_code);
}
