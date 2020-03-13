#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <thread>
#include <functional>
#include <string>

typedef std::pair<double, double> PAIR_DD;
typedef std::function<double(const double&, const double&)> FUNC;

struct config
{
    double abs_error;
    double rel_error;
    size_t threads_count;
    PAIR_DD x_range, y_range;
};

config read_config(std::ifstream& fs);
double f(const double x1, const double x2);
template <typename F>
void integral(const F& func, config cfg, const PAIR_DD& delta, double& value);
template <typename F>
double integral_parallel(const F& func, config cfg, const PAIR_DD& delta);
PAIR_DD thread_domain(const PAIR_DD& range, const int thread_id, const size_t threads_count);

inline double abs_error(const double a, const double b) { return std::abs(a - b); }
inline double rel_error(const double a, const double b) { return abs_error(a, b) / std::max(std::abs(a), std::abs(b)); }

// https://cms.ucu.edu.ua/mod/page/view.php?id=103097
inline std::chrono::high_resolution_clock::time_point get_current_time_fenced();
template<class D>
inline long long to_us(const D& d);

template <typename F>
long long time_integral(const F& func, const config& cfg, size_t steps, double& value);

int main(int argc, char** argv)
{
    const std::string FILE_OUT = "out.txt";

    if (argc != 2)
    {
        std::cout << "Wrong amount of arguments. Given " << argc - 1 << ", required 1" << std::endl;
        return -1;
    }

    config c;
    std::ifstream fs(argv[1], std::ifstream::in);

    if ( !fs.is_open() )
    {
        std::cout << "File " << argv[1] << " is not found" << std::endl;
        return -1;
    }

    c = read_config(fs);
    fs.close();

    size_t steps = 100;
    double val = 0;
    long long calc_time = time_integral(f, c, steps, val);
    std::cout << calc_time << std::endl;

    std::ofstream fsout(FILE_OUT, std::ofstream::app);
    fsout << std::fixed << val << std::endl << calc_time << std::endl;
    fsout.close();

    std::cout << "The results were appended to " << FILE_OUT << " file" << std::endl;

    return 0;
}

config read_config(std::ifstream& fs)
{
    config cfg;
    fs >> cfg.abs_error >>
          cfg.rel_error >>
          cfg.threads_count >>
          cfg.x_range.first >>
          cfg.x_range.second >>
          cfg.y_range.first >>
          cfg.y_range.second;
    return cfg;
}

PAIR_DD thread_domain(const PAIR_DD& range, const int thread_id, const size_t threads_count)
{
    double range_size = range.second - range.first,
            thread_domain_size = range_size / threads_count;
    return PAIR_DD(range.first + thread_id * thread_domain_size, range.first + (1 + thread_id) * thread_domain_size);
}

double f(const double x1, const double x2)
{
    double sum = 0;
    for (int i = -2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
            sum += 1 / (5 * (i + 2) + j + 3 + std::pow(x1 - 16 * j, 6) + std::pow(x2 - 16 * i, 6));
        }
    }
    return 1 / (0.002 + sum);
}

template <typename F>
void integral(const F& func, config cfg, const PAIR_DD& delta, double& value)
{
    double sum = 0;
    double delta_area = delta.first * delta.second,
            half_delta_x = delta.first / 2,
            half_delta_y = delta.second / 2;
    for (double x = cfg.x_range.first; x < cfg.x_range.second; x += delta.first)
    {
        for (double y = cfg.y_range.first; y < cfg.y_range.second; y += delta.second)
        {
            sum += func(x + half_delta_x, y + half_delta_y);
        }
    }
    value = sum * delta_area;
}

template <typename F>
double integral_parallel(const F& func, config cfg, const PAIR_DD& delta)
{
    double integral_sum = 0;
    std::vector<std::thread> threads;
    std::vector<double> integral_parts(cfg.threads_count, 0);
    PAIR_DD y_domain, y_fullrange = cfg.y_range;
    for (int thread_id = 0; thread_id < static_cast<int>(cfg.threads_count); thread_id++)
    {
        y_domain = thread_domain(y_fullrange, thread_id, cfg.threads_count);
        cfg.y_range = y_domain;
        threads.emplace_back(integral<FUNC>, std::cref(func), cfg, std::cref(delta), std::ref(integral_parts[thread_id]));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    for (const auto val : integral_parts)
    {
        integral_sum += val;
    }

    return integral_sum;
}

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

template <typename F>
long long time_integral(const F& func, const config& cfg, size_t steps, double& value)
{
    auto start_time = get_current_time_fenced();

    double prev_val;

    value = integral_parallel(func, cfg, PAIR_DD((cfg.x_range.second - cfg.x_range.first) / steps,
                                                 (cfg.y_range.second - cfg.y_range.first) / steps));
    steps *= 2;

    do
    {
        prev_val = value;
        value = integral_parallel(func, cfg, PAIR_DD((cfg.x_range.second - cfg.x_range.first) / steps,
                                                     (cfg.y_range.second - cfg.y_range.first) / steps));
        steps *= 2;
    } while(abs_error(value, prev_val) > cfg.abs_error || rel_error(value, prev_val) > cfg.rel_error);

    auto end_time = get_current_time_fenced();
    return to_us(end_time - start_time);
}
