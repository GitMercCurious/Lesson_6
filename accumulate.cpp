#include <iostream>
#include <thread>
#include <atomic>
#include <numeric>
#include <vector>
#include <algorithm>
#include <random>

// 1. Проверить размер. Если он мал, то не распараллеливаем.
// 2. Определить количество исполнителей -- спросить у ОС.
// 3. Распределить данные между исполнителями.
// 4. Запустить исполнение.
// 5. Собрать ответы.

// Функция, которую будем передавать в потоки.
// Мы не можем получить оттуда возвращаемое значение через return,
// поэтому будем записывать его в переменную result по ссылке
template <typename Iterator, typename T>
void accumulate_wrapper(Iterator begin, Iterator end, T init, std::atomic<T>& result) {
    result += std::accumulate(begin, end, init);
    // DONE
}

template <typename Iterator, typename T>
T parallel_accumulate(Iterator begin, Iterator end, T init) {
    // Размер диапазона
    auto size = std::distance(begin, end);
    // Количество исполнителей
    auto num_workers = std::thread::hardware_concurrency();
    // Проверяем, нужно ли распараллеливать
    if (size < num_workers * 4) {
        return std::accumulate(begin, end, init);
    }
    // Распараллеливать всё-таки нужно, тогда:
    // Считаем количество данных на одного исполнителя
    auto size_per_worker = size / num_workers;
    // Создаём два вектора: с потоками и для записи результатов
    std::vector<std::thread> threads;
    threads.reserve(num_workers - 1);
    std::atomic<T> result = init;
    // Распределяем данные и запускаем потоки
    // (на 1 меньше максимума, так как ещё есть основной поток).
    for(auto i = 0u; i < num_workers - 1; i++) {
        threads.push_back(std::thread(accumulate_wrapper<Iterator, T>,
                                      std::next(begin, i * size_per_worker), // сдвиг begin
                                      std::next(begin, (i + 1) * size_per_worker), // сдвиг begin
                                      0, // init
                                      std::ref(result))); // для записи результата
    }
    // Производим расчёт и в основном потоке (с учётом переданного init)
    result += std::accumulate(std::next(begin, (num_workers - 1) * size_per_worker), end, 0);

    // Ждём, пока остальные потоки завершат работу
    std::for_each(std::begin(threads),std::end(threads),std::mem_fun_ref(&std::thread::join));
    // DONE
    // Собираем все вычисленные результаты
    return result;
}

int main() {
    // Считаем сумму чисел от 0 до 99
    std::vector<int> sequence(100);
    std::iota(std::begin(sequence), std::end(sequence), 0);
    std::cout << parallel_accumulate(std::begin(sequence),
                                     std::end(sequence),
                                     0)
              << std::endl << std::endl;

    // TODO: Сгенерируйте вектор случайных чисел,
    // используя инструментарий из библиотеки random,
    // и вычислите их сумму с помощью функции
    // parallel_accumulate.

    std::vector<int> random_seq(1000, 0);
    unsigned long long seed = static_cast<unsigned long long>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937_64 eng(seed);
    std::uniform_int_distribution<int> distribution(0, 1000);

    for(auto &el: random_seq){
        el = distribution(eng);
        std::cout << el << ' ';
    }

    std::cout << std::endl << std::endl;
    std::cout << parallel_accumulate(std::begin(random_seq),
                                     std::end(random_seq),
                                     0)
              << std::endl;
}