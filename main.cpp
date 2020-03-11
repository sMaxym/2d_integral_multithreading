#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <thread>
#include <functional>

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

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return -1;
    }

    config c;
    std::ifstream fs(argv[1], std::ifstream::in);
    c = read_config(fs);
    fs.close();

    size_t steps = 100;
    double value = 0,
            prev_val = 0;

    do
    {

        prev_val = value;
        value = integral_parallel(f, c, PAIR_DD((c.x_range.second - c.x_range.first) / steps,
                                                (c.y_range.second - c.y_range.first) / steps));
        steps *= 2;

        std::cout << value << " " << prev_val << " " << abs_error(value, prev_val) << " " << rel_error(value, prev_val) << " " << steps / 2 << std::endl;

    } while(abs_error(value, prev_val) > c.abs_error || rel_error(value, prev_val) > c.rel_error);


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
    for (double x = cfg.x_range.first; x < cfg.x_range.second; x += delta.first)
    {
        for (double y = cfg.y_range.first; y < cfg.y_range.second; y += delta.second)
        {
            sum += func(x, y) * delta.first * delta.second;
        }
    }
    value = sum;
}

template <typename F>
double integral_parallel(const F& func, config cfg, const PAIR_DD& delta)
{
    double integral_sum = 0;
    std::vector<std::thread> threads;
    std::vector<double> integral_parts(cfg.threads_count, 0);
    PAIR_DD y_domain, y_fullrange = cfg.y_range;
    for (int thread_id = 0; thread_id < (int)cfg.threads_count; thread_id++)
    {
        y_domain = thread_domain(y_fullrange, thread_id, cfg.threads_count);
        cfg.y_range = y_domain;
        threads.emplace_back(integral<FUNC>, std::cref(func), cfg, std::cref(delta), std::ref(integral_parts[thread_id]));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    for (auto val : integral_parts)
    {
        integral_sum += val;
    }

    return integral_sum;
}
