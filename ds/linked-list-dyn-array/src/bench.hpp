#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <iostream>

#include "dynamic-array.hpp"
#include "singly-linked-list.hpp"
#include "doubly-linked-list.hpp"

constexpr int SEED = 280131;

class Benchmark {
private:
    static constexpr size_t WARMUP_ITERATIONS = 10;
    static constexpr size_t TEST_ITERATIONS = 50;

    struct TestResult {
        std::string structure;
        std::string operation;
        std::string position;
        size_t elements;
        double avg_time_ns;
        double std_deviation;
        double min_time_ns;
        double max_time_ns;
    };

    template<typename F>
    static TestResult measure(
        F&& fn,
        const std::string& structure_name,
        const std::string& operation,
        const std::string& position,
        size_t elements
    ) {
        std::vector<double> measurements;
        measurements.reserve(TEST_ITERATIONS);

        for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
            fn();
        }

        for (size_t i = 0; i < TEST_ITERATIONS; ++i) {
            const auto start = std::chrono::high_resolution_clock::now();
            fn();
            const auto end = std::chrono::high_resolution_clock::now();

            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            measurements.push_back(duration.count());
        }

        double sum = 0;
        double min = measurements[0];
        double max = measurements[0];

        for (double time : measurements) {
            sum += time;
            min = std::min(min, time);
            max = std::max(max, time);
        }

        double avg = sum / TEST_ITERATIONS;

        double variance = 0;
        for (double time : measurements) {
            variance += (time - avg) * (time - avg);
        }
        double std_dev = std::sqrt(variance / TEST_ITERATIONS);

        return {
            structure_name,
            operation,
            position,
            elements,
            avg,
            std_dev,
            min,
            max
        };
    }

    static void write_results(const std::vector<TestResult>& results) {
        std::ofstream file("benchmark_results.csv");
        file << "Structure,Operation,Position,Elements,"
            << "Average(ns),StdDev(ns),Min(ns),Max(ns)\n";

        std::map<std::string, std::vector<TestResult>> grouped_results;
        for (const auto& result : results) {
            grouped_results[result.structure].push_back(result);
        }

        for (const auto& [structure, structure_results] : grouped_results) {
            file << "\n" << structure << "\n";
            for (const auto& result : structure_results) {
                file << result.structure << ","
                    << result.operation << ","
                    << result.position << ","
                    << result.elements << ","
                    << std::fixed << std::setprecision(2)
                    << result.avg_time_ns << ","
                    << result.std_deviation << ","
                    << result.min_time_ns << ","
                    << result.max_time_ns << "\n";
            }
        }

        file.close();

        std::cout << "\nBenchmark Summary:\n";
        for (const auto& [structure, structure_results] : grouped_results) {
            std::cout << "\n" << structure << ":\n";
            for (const auto& result : structure_results) {
                std::cout << "  " << std::setw(10) << result.operation
                    << " " << std::setw(8) << result.position
                    << " (n=" << result.elements << "): "
                    << std::fixed << std::setprecision(2)
                    << result.avg_time_ns << " ns ±"
                    << result.std_deviation << " ns\n";
            }
        }
    }

public:
    static void run(const std::vector<size_t>& test_sizes) {
        std::cout << "Starting benchmark suite...\n";
        std::vector<TestResult> all_results;

        for (size_t elements : test_sizes) {
            std::cout << "\nTesting with " << elements << " elements\n";

            benchmark_structure<DynamicArray<int>>(all_results, "DynamicArray", elements);
            benchmark_structure<SinglyLinkedList<int>>(all_results, "SinglyLinkedList", elements);
            benchmark_structure<DoublyLinkedList<int>>(all_results, "DoublyLinkedList", elements);
        }

        std::cout << "\n\nWriting results to file...\n";
        write_results(all_results);
        std::cout << "Benchmark completed! Results written to 'benchmark_results.csv'\n";
    }

private:
    template<typename S>
    static void benchmark_structure(
        std::vector<TestResult>& results,
        const std::string& name,
        size_t elements
    ) {
        // seed for deterministic results
        std::mt19937 gen(SEED);
        std::uniform_int_distribution<> val_dis(0, 1000000);

		std::cout << "Push Front" << "...\n";
        {
            S s {};
			const auto result = measure(
				[&]() { s.push_front(val_dis(gen)); },
				name, "Push", "Front", elements
			);
            results.push_back(result);
        }

        {
            S s {};
			const auto result = measure(
				[&]() { s.push_back(val_dis(gen)); },
				name, "Push", "Back", elements
			);
            results.push_back(result);
        }

        {
            S s {};
            // pre-fill
            for (size_t i = 0; i < elements / 2; ++i) {
                s.push_back(val_dis(gen));
            }
            const auto result = measure(
                [&]() { s.insert(val_dis(gen), s.size() / 2); },
                name, "Push", "Middle", elements
            );
            results.push_back(result);
        }

        {
            S s {};
            // pre-fill
            for (size_t i = 0; i < elements; ++i) {
                s.push_back(val_dis(gen));
            }
            const auto result = measure(
                [&]() { 
                    if (s.empty()) s.push_back(val_dis(gen));
                    s.pop_front(); 
                },
                name, "Pop", "Front", elements
            );
            results.push_back(result);
        }

        {
            S s {};
            // pre-fill
            for (size_t i = 0; i < elements; ++i) {
                s.push_back(val_dis(gen));
            }
            const auto result = measure(
                [&]() { 
                    if (s.empty()) s.push_back(val_dis(gen));
                    s.pop_back(); 
                },
                name, "Pop", "Back", elements
            );
            results.push_back(result);
        }

        {
            S s {};
            // pre-fill
            for (size_t i = 0; i < elements; ++i) {
                s.push_back(val_dis(gen));
            }
            const auto result = measure(
                [&]() {
                    size_t middle = s.size() / 2;
                    if (middle < s.size()) {
                        s.remove(middle);
                    }
                },
                name, "Remove", "Middle", elements
            );
            results.push_back(result);
        }

        {
            S s {};
            // pre-fill
            for (size_t i = 0; i < elements; ++i) {
                s.push_back(val_dis(gen));
            }
            const auto result = measure(
                [&]() { s.find(val_dis(gen)); },
                name, "Find", "Random", elements
            );
            results.push_back(result);
        }

        std::cout << "." << std::flush;
    }
};