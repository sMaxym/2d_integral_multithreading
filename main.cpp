#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <utility>
#include <thread>

struct config
{
    double abs_error;
    double rel_error;
    unsigned int threads_count;
    std::pair<double, double> x_range, y_range;
};

config read_config(std::ifstream& fs);
double f(const double x1, const double x2);
template <typename F>
double integral(const F& func, const config& cfg, const std::pair<double, double>& delta);
template <typename F>
double estimate_integral(const F& func, const config& cfg, unsigned int div = 500);

inline double abs_error(const double a, const double b) { return std::abs(a - b); }
inline double rel_error(const double a, const double b) { return abs_error(a, b) / b; }

int main(int argc, char** argv)
{
    config c;
    std::ifstream fs(argv[1], std::ifstream::in);
    c = read_config(fs);
    fs.close();



    return 0;
}

config read_config(std::ifstream& fs)
{
    config cfg;
    fs >> cfg.abs_error >> cfg.rel_error >> cfg.threads_count >> cfg.x_range.first >> cfg.x_range.second >> cfg.y_range.first >> cfg.y_range.second;
    return cfg;
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
double integral(const F& func, const config& cfg, const std::pair<double, double>& delta)
{
    double sum = 0;
    for (double x = cfg.x_range.first; x < cfg.x_range.second; x += delta.first)
    {
        for (double y = cfg.y_range.first; y < cfg.y_range.second; y += delta.second)
        {
            sum += func(x, y) * delta.first * delta.second;
        }
    }
    return sum;
}

template <typename F>
double estimate_integral(const F& func, const config& cfg, unsigned int div)
{
    double s1, s2;
    double x_size = (cfg.x_range.second - cfg.x_range.first),
           y_size = (cfg.y_range.second - cfg.y_range.first);
    do
    {
        s1 = integral(func, cfg, std::pair<double, double>(x_size / div, y_size / div));
        div *= 2;
        s2 = integral(func, cfg, std::pair<double, double>(x_size / div, y_size / div));
        std::cout << s1 << " " << s2 << " " << abs_error(s1, s2) << " " << rel_error(s1, s2) << " " << std::endl;
    } while(abs_error(s1, s2) > cfg.abs_error || rel_error(s1, s2) > cfg.rel_error);
    return s2;
}
