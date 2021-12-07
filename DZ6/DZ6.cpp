#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

/*
  Задание 1.
 Создайте потокобезопасную оболочку для объекта cout. 
 Назовите ее pcout.
 Необходимо, чтобы несколько потоков могли обращаться к pcout и информация выводилась в консоль.
 Продемонстрируйте работу pcout.
    
  Задание 2.
 Реализовать функцию, возвращающую i-ое простое число (например, миллионное простое число равно 15485863).
 Вычисления реализовать во вторичном потоке. В консоли отображать прогресс вычисления.
        
  Задание 3.
 Промоделировать следующую ситуацию.
 Есть два человека (2 потока): хозяин и вор.
 Хозяин приносит домой вещи (функция добавляющая случайное число в вектор с периодичностью 1 раз в секунду).
 При этом у каждой вещи есть своя ценность. 
 Вор забирает вещи (функция, которая находит наибольшее число и удаляет из вектора с периодичностью 1 раз в 0.5 секунд),
 каждый раз забирает вещь с наибольшей ценностью. */

using namespace std;

const vector<string> THINGS{ "Gold Coin", "Silver Coin", "Watch", "Trash", "Computer", "Videocard" };

int getRandomNum(int min, int max)
{
    const static auto seed = chrono::system_clock::now().time_since_epoch().count();
    static mt19937_64 generator(seed);
    uniform_int_distribution<int> dis(min, max);
    return dis(generator);
}

mutex m_pcout;
void pcout(const string& text)
{
    m_pcout.lock();
    this_thread::sleep_for(chrono::duration<int, deci>(getRandomNum(10, 20)));
    cout << text << endl;
    m_pcout.unlock();
}

mutex m_simple;
int FindSimple(int* counter, const int simple_number)
{
    pcout("Calculate started");
    vector<int> simple_list{ 2 };
    simple_list.reserve(simple_number);
    int num = 1;
    do {
        num += 2;
        bool b_is_simple = true;

        for (auto simple : simple_list)
        {
            if ((num % simple) == 0) 
            {
                b_is_simple = false;
                break;
            }
        }

        if (b_is_simple) 
        {
            m_simple.lock();
            (*counter)++;
            m_simple.unlock();
            simple_list.push_back(num);
        }

    } while (*counter < simple_number);
    pcout(to_string(simple_list.at(simple_number - 1)));

    return simple_list.at(simple_number - 1);
}

void PrintProgress(const int* counter, const int simple_number)
{
    do {
        m_simple.lock();
        pcout("Progress:" + to_string(*counter) + "/" + to_string(simple_number));
        m_simple.unlock();
        this_thread::sleep_for(chrono::duration<int, deci>(1));
    } while (*counter < simple_number);
    m_simple.lock();
    pcout("Progress:" + to_string(*counter) + "/" + to_string(simple_number));
    m_simple.unlock();
}

mutex m_thief;
void BringThingToHome(vector<pair<string, int>>* storage)
{
    int counter = 0;
    do {
        pair new_thing = { THINGS[getRandomNum(0, int(THINGS.size() - 1))], getRandomNum(0, 1000) };
        m_thief.lock();
        storage->push_back(new_thing);
        pcout("Bring thing:" + new_thing.first + " with price:" + to_string(new_thing.second));
        m_thief.unlock();
        this_thread::sleep_for(chrono::duration<int, deci>(1));
        counter++;

    } while (counter < 1000);
}

void StealThingFromHome(vector<pair<string, int>>* storage) 
{
    this_thread::sleep_for(chrono::duration<int, deci>(150));
    int counter = 0;
    do {
        int highest_price_index = -1;
        m_thief.lock();
        for (int i = 0; i < storage->size(); i++)
        {
            if (highest_price_index == -1 || storage->at(i).second > storage->at(highest_price_index).second)
            {
                highest_price_index = i;
            }
        }
        if (highest_price_index >= 0)
        {
            pcout("Steal thing:" + storage->at(highest_price_index).first + " with price:" + to_string(storage->at(highest_price_index).second));
            storage->erase(storage->begin() + highest_price_index);
        }

        m_thief.unlock();
        this_thread::sleep_for(chrono::duration<int, centi>(5));
        counter++;

    } while (counter < 500);
}

void T_One()
{
    thread TP1{ pcout, "Test string number 1" };
    thread TP2{ pcout, "Test string number 2" };
    thread TP3{ pcout, "Test string number 3" };
    TP1.join();
    TP2.join();
    TP3.join();
}

void T_Two()
{
    int progress = 1;
    int result;
    int simple_number;
    cout << "Enter number to find simple:";
    cin >> simple_number;
    thread progress_thread{ PrintProgress, &progress, simple_number };
    progress_thread.detach();
    thread simple_thread([&]() { result = FindSimple(&progress, simple_number); });
    simple_thread.join();
 
}

void T_Free()
{
    vector<pair<string, int>> storage;
    thread host{ BringThingToHome, &storage };
    host.detach();
    thread thief{ StealThingFromHome, &storage };
    thief.join();
}

int main()
{
    cout << "\nTask 1: \n" << endl;
    T_One();

    cout << "\nTask 2: \n" << endl;
    T_Two();

    cout << "\nTask 3: \n" << endl;
    T_Free();

    return 0;
}
