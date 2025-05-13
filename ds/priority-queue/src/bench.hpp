#pragma once

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

template<typename Context>
class BenchmarkTest {
public:
    using SetupFn = std::function<Context(size_t)>;
    using TestFn = std::function<void(Context&, size_t)>;
    using PostFn = std::function<void(Context&)>;

    std::string name;
    size_t elements;
    SetupFn setup;
    TestFn test;
    PostFn post;

    BenchmarkTest(
        const std::string& name,
        const size_t elements,
        const SetupFn setup_fn,
        const TestFn test_fn,
        const PostFn post_fn = [](Context&) {}
    ) : name(name),
        elements(elements),
        setup(setup_fn),
        test(test_fn),
        post(post_fn) {
    }
};

class BenchmarkSuite {
public:
    struct TestResult {
        std::string name;
        size_t elements;
        double avg_time_ns;
        double std_deviation;
        size_t samples_used;
    };

    size_t warmup_iterations;
    size_t test_iterations;
    size_t batch_iterations;
    std::vector<TestResult> results;

    BenchmarkSuite(
        const size_t warmup_iterations = 50,
        const size_t test_iterations = 300,
        const size_t batch_iterations = 1
    ) : warmup_iterations(warmup_iterations),
        test_iterations(test_iterations),
        batch_iterations(batch_iterations) {}

    double calculate_quartile(std::vector<double>& data, double q) {
        const size_t n = data.size();
        const double pos = q * (n - 1);
        const size_t ind = static_cast<size_t>(pos);
        const double fraction = pos - ind;
        
        if (ind + 1 < n) {
            return data[ind] * (1.0 - fraction) + data[ind + 1] * fraction;
        }
        return data[ind];
    }

    template<typename Context>
    TestResult run_test(const BenchmarkTest<Context>& test) {
        std::cout << "Running " << test.name << " with " << test.elements << " elements...\n";

        std::vector<double> measurements;
        measurements.reserve(test_iterations);

        // Warmup phase
        for (size_t i = 0; i < warmup_iterations; i++) {
            Context context = test.setup(i);
            for (size_t j = 0; j < batch_iterations; j++) {
                test.test(context, j);
            }
            test.post(context);
        }

        // Measurement phase
        for (size_t i = 0; i < test_iterations; i++) {
            Context context = test.setup(i);
            
            const auto start = std::chrono::high_resolution_clock::now();
            for (size_t j = 0; j < batch_iterations; j++) {
                test.test(context, j);
            }
            const auto end = std::chrono::high_resolution_clock::now();

            test.post(context);

            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            measurements.push_back(duration.count() / batch_iterations);
        }

        // Sort measurements for quartile calculation
        std::sort(measurements.begin(), measurements.end());

        // Calculate quartiles
        const double q1 = calculate_quartile(measurements, 0.25);
        const double q3 = calculate_quartile(measurements, 0.75);
        const double iqr = q3 - q1;
        const double lower_bound = q1 - 1.5 * iqr;
        const double upper_bound = q3 + 1.5 * iqr;

        // Filter outliers
        std::vector<double> filtered_measurements;
        filtered_measurements.reserve(measurements.size());
        
        for (const double time : measurements) {
            if (time >= lower_bound && time <= upper_bound) {
                filtered_measurements.push_back(time);
            }
        }

        // Calculate statistics on filtered data
        double sum = 0;
        for (double time : filtered_measurements) {
            sum += time;
        }

        const double avg = sum / filtered_measurements.size();

        double variance = 0;
        for (double time : filtered_measurements) {
            variance += (time - avg) * (time - avg);
        }
        const double std_dev = std::sqrt(variance / filtered_measurements.size());

        TestResult result {
            test.name,
            test.elements,
            avg,
            std_dev,
            filtered_measurements.size()
        };

        results.push_back(result);
        return result;
    }

    template<typename Context>
    void run_tests(const std::vector<BenchmarkTest<Context>>& tests) {
        for (const auto& test : tests) {
            run_test(test);
        }
    }

    void write_results(const std::string& filename = "benchmark_results.csv") const {
        std::ofstream file(filename);
        file << "Algorithm,Elements,Average(ns),StdDev(ns),SamplesUsed\n";

        std::map<std::string, std::vector<TestResult>> grouped_results;
        for (const auto& result : results) {
            grouped_results[result.name].push_back(result);
        }

        for (const auto& [name, algorithm_results] : grouped_results) {
            for (const auto& result : algorithm_results) {
                file << result.name << ","
                    << result.elements << ","
                    << std::fixed << std::setprecision(2)
                    << result.avg_time_ns << ","
                    << result.std_deviation << ","
                    << result.samples_used
                    << "\n";
            }
        }

        file.close();

        std::cout << "\nBenchmark Summary:\n";
        for (const auto& [name, algorithm_results] : grouped_results) {
            std::cout << "\n" << name << ":\n";
            for (const auto& result : algorithm_results) {
                std::cout
                    << " (n=" << result.elements << ", samples=" << result.samples_used << "): "
                    << std::fixed << std::setprecision(2)
                    << result.avg_time_ns << " ns +- "
                    << result.std_deviation << " ns\n";
            }
        }
    }
};
