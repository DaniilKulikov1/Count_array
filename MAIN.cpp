#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>


class Array
{
private:
    int size_arr; // размер
    int* arr; //указатель

public:
    //мои конструктора
    Array() : size_arr(0), arr(nullptr) {}; //по-умолчанию размер массива 0, указатель - 0
Array(int n) : size_arr(n), arr(new int[n]) {} // если в ручную - то n c клавиатуры, и выделяем память под массив n элементов


~Array()//деструктор
{
    if (arr != nullptr) //если указатель не 0 
    {
        delete[] arr; //очистить его
        //когда всё кончится этот деструктор очитит
    }
}

//рразмер массиваы равен введённому с клавиатуры
void set_size_arr(int n)
{ size_arr = n; }

//заполнение массива
void fill_array(int n)
{
    //если указатель 0
    if (arr == nullptr) 
    {
        arr = new int[n];
        //указатель делаем равным введёному числу
    }

    for (int i = 0; i < n; i++) 
    {
        //заполняем массив с первого по последее звено случайными числами
        arr[i] = rand() % n;
    }
}

//вернё  указатель на массив
    int* get_array() 
    { return arr; }

};


//сделаем атомарный обьект сразу равным 0
std::atomic<int> sum(0);


void calc_summ_of_piece(const int* arr, const int size_of_arr, const int size_of_piece, const int n)
{
    int local_sum(0); // Инициализируем локальную переменную для хранения суммы элементов в текущем диапазоне
    int array_begin = n * size_of_piece;// Вычисляем начальный индекс диапазона, который будет обрабатывать текущий поток
    if (array_begin >= size_of_arr)
    {// Если начальный индекс больше или равен размеру массива, то выходим из функции
        return;
    }

    int array_end = array_begin + size_of_piece; // Вычисляем конечный индекс диапазона, который будет обрабатывать текущий поток
    if (array_end > size_of_arr) 
    {// Если конечный индекс выходит за пределы массива, то ограничиваем его размером массива
        array_end = size_of_arr;
    }

    // Проходим по диапазону элементов, которые должен обработать текущий поток
    for (int i = array_begin; i < array_end; i++)
    {
        local_sum += arr[i];
        // Суммируем элементы в текущем диапазоне
    }
    sum += local_sum;// Добавляем локальную сумму к общей сумме, защищенной атомарным объектом
}

int calculate_using_threads(int* array, const int size, const int counts_threads)
{
  
    std::vector<std::thread> threads;//создаём вектор потоков

    int counts_items = size / counts_threads;  // Вычисляем количество элементов, которые будет обрабатывать каждый поток
    int remainder = size - (counts_items * counts_threads);// Вычисляем остаток, который не поделился на количество потоков
  
    
    // Если остаток больше 1, то увеличиваем количество элементов, обрабатываемых каждым потоком,
    // чтобы распределить остаток равномерно между всеми потоками
    while (remainder > 1) {
        counts_items++;
        remainder = size - (counts_items * counts_threads);
    }

    // Запускаем потоки, каждый из которых будет вычислять сумму элементов своего диапазона
    for (int i = 0; i < counts_threads; i++) 
    {

        // Создаем новый поток, передавая ему массив, размер массива, количество элементов,
        // обрабатываемых каждым потоком, и номер текущего потока
        std::thread t(calc_summ_of_piece, array, size, counts_items, i);
        threads.push_back(std::move(t)); // Добавляем созданный поток в вектор потоков
    }

    for (auto& t : threads) // Ожидаем завершения всех запущенных потоков
    {t.join();}
    // Возвращаем итоговую сумму, вычисленную всеми потоками
    return sum;
}

int calculate_without_threads(int* array, const int size) 
{
    int local_sum(0);// Инициализируем локальную переменную для хранения суммы элементов
    for (int i = 0; i < size; i++)
    {
        // Суммируем элементы массива
        local_sum += array[i];
    }
    return local_sum;// Возвращаем итоговую сумму
}

int main() 

{
 
    setlocale(LC_ALL, "ru");

    std::cout << "введите размер вашего массива: ";
    int size(0);
    std::cin >> size;
    Array array(size);
    array.fill_array(size);

    std::cout << "Введите количество потоков: ";
    int counts_threads(0);
    std::cin >> counts_threads;

    
    auto t_start = std::chrono::system_clock::now();
    int sum = calculate_using_threads(array.get_array(), size, counts_threads);
    auto t_end = std::chrono::system_clock::now();
    long diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    std::cout << "Использовано потоков: " << counts_threads << " сумма элементов массива: " << sum
        << "  время (милисекунды): " << diff << std::endl;


    t_start = std::chrono::system_clock::now();
    sum = calculate_without_threads(array.get_array(), size);
    t_end = std::chrono::system_clock::now();
    diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    std::cout << "Без потоков сумма элементов массива: " << sum << " время (милисекунды): " << diff << std::endl;

    return 0;
}