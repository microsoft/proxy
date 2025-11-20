// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

struct Environment {
  std::string Description;
  std::string InfoPath;
  std::string BenchmarkingResultsPath;

  friend void to_json(nlohmann::json& j, const Environment& e) {
    j = nlohmann::json{
        {"Description", e.Description},
        {"InfoPath", e.InfoPath},
        {"BenchmarkingResultsPath", e.BenchmarkingResultsPath},
    };
  }

  friend void from_json(const nlohmann::json& j, Environment& e) {
    j.at("Description").get_to(e.Description);
    j.at("InfoPath").get_to(e.InfoPath);
    j.at("BenchmarkingResultsPath").get_to(e.BenchmarkingResultsPath);
  }
};

struct EnvironmentInfo {
  std::string OS;
  std::string KernelVersion;
  std::string Architecture;
  std::string Compiler;

  friend void to_json(nlohmann::json& j, const EnvironmentInfo& e) {
    j = nlohmann::json{
        {"OS", e.OS},
        {"KernelVersion", e.KernelVersion},
        {"Architecture", e.Architecture},
        {"Compiler", e.Compiler},
    };
  }

  friend void from_json(const nlohmann::json& j, EnvironmentInfo& e) {
    j.at("OS").get_to(e.OS);
    j.at("KernelVersion").get_to(e.KernelVersion);
    j.at("Architecture").get_to(e.Architecture);
    j.at("Compiler").get_to(e.Compiler);
  }
};

struct Metric {
  std::string Name;
  std::string TargetBenchmarkName;
  std::string BaselineBenchmarkName;

  friend void to_json(nlohmann::json& j, const Metric& m) {
    j = nlohmann::json{
        {"Name", m.Name},
        {"TargetBenchmarkName", m.TargetBenchmarkName},
        {"BaselineBenchmarkName", m.BaselineBenchmarkName},
    };
  }

  friend void from_json(const nlohmann::json& j, Metric& m) {
    j.at("Name").get_to(m.Name);
    j.at("TargetBenchmarkName").get_to(m.TargetBenchmarkName);
    j.at("BaselineBenchmarkName").get_to(m.BaselineBenchmarkName);
  }
};

struct MetricGroup {
  std::string Name;
  std::string Description;
  std::vector<Metric> Metrics;

  friend void to_json(nlohmann::json& j, const MetricGroup& m) {
    j = nlohmann::json{
        {"Name", m.Name},
        {"Description", m.Description},
        {"Metrics", m.Metrics},
    };
  }

  friend void from_json(const nlohmann::json& j, MetricGroup& m) {
    j.at("Name").get_to(m.Name);
    j.at("Description").get_to(m.Description);
    j.at("Metrics").get_to(m.Metrics);
  }
};

struct ReportConfig {
  std::string TargetName;
  double YellowIndicatorThreshold;
  std::string OutputPath;
  std::vector<Environment> Environments;
  std::vector<MetricGroup> MetricGroups;

  friend void to_json(nlohmann::json& j, const ReportConfig& rc) {
    j = nlohmann::json{
        {"TargetName", rc.TargetName},
        {"YellowIndicatorThreshold", rc.YellowIndicatorThreshold},
        {"OutputPath", rc.OutputPath},
        {"Environments", rc.Environments},
        {"MetricGroups", rc.MetricGroups},
    };
  }

  friend void from_json(const nlohmann::json& j, ReportConfig& rc) {
    j.at("TargetName").get_to(rc.TargetName);
    j.at("YellowIndicatorThreshold").get_to(rc.YellowIndicatorThreshold);
    j.at("OutputPath").get_to(rc.OutputPath);
    j.at("Environments").get_to(rc.Environments);
    j.at("MetricGroups").get_to(rc.MetricGroups);
  }
};

const std::string_view MedianSuffix = "_median";

EnvironmentInfo ParseEnvironmentInfo(const std::filesystem::path& file) {
  nlohmann::json obj;
  std::ifstream in;
  in.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  in.open(file, std::ios_base::in | std::ios_base::binary);
  in >> obj;
  return obj.get<EnvironmentInfo>();
}

std::unordered_map<std::string, double>
    ParseBenchmarkingResults(const std::filesystem::path& file) {
  nlohmann::json obj;
  {
    std::ifstream in;
    in.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    in.open(file, std::ios_base::in | std::ios_base::binary);
    in >> obj;
  }
  std::unordered_map<std::string, double> result;
  for (auto& node : obj["benchmarks"]) {
    std::string name = node["name"];
    if (name.ends_with(MedianSuffix)) {
      name.resize(name.size() - MedianSuffix.size());
      double value = node["real_time"];
      result.emplace(std::move(name), value);
    }
  }
  return result;
}

void GenerateReport(const std::filesystem::path& config_path) {
  ReportConfig config;
  {
    nlohmann::json obj;
    std::ifstream in;
    in.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    in.open(config_path, std::ios_base::in | std::ios_base::binary);
    in >> obj;
    obj.get_to(config);
  }
  std::vector<std::unordered_map<std::string, double>> benchmarks;
  benchmarks.reserve(config.Environments.size());
  for (auto& environment : config.Environments) {
    benchmarks.push_back(
        ParseBenchmarkingResults(environment.BenchmarkingResultsPath));
  }
  std::ofstream out;
  out.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  out.open(config.OutputPath,
           std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  for (auto& metric_group : config.MetricGroups) {
    out << "## " << metric_group.Name << "\n\n";
    out << metric_group.Description << "\n\n";
    out << "| |";
    for (auto& environment : config.Environments) {
      out << " " << environment.Description << " |";
    }
    out << "\n";
    out << "| - |";
    for (std::size_t i = 0; i < config.Environments.size(); ++i) {
      out << " - |";
    }
    out << "\n";
    for (auto& metric : metric_group.Metrics) {
      out << "| " << metric.Name << " |";
      for (auto& benchmark : benchmarks) {
        double target = benchmark.at(metric.TargetBenchmarkName);
        double baseline = benchmark.at(metric.BaselineBenchmarkName);
        double rate = (baseline - target) * 100 / target;
        bool is_negative = rate < 0;
        if (is_negative) {
          rate = -rate;
        }
        out << " ";
        if (rate < config.YellowIndicatorThreshold) {
          out << "\xf0\x9f\x9f\xa1"; // Yellow circle
        } else if (is_negative) {
          out << "\xf0\x9f\x94\xb4"; // Red circle
        } else {
          out << "\xf0\x9f\x9f\xa2"; // Green circle
        }
        auto rate_str = std::format("{:.1f}", rate);
        std::string message;
        if (rate_str == "0.0") {
          out << config.TargetName << " has similar performance";
        } else {
          out << config.TargetName << " is about **" << rate_str << "% "
              << (is_negative ? "slower" : "faster") << "**";
        }
        out << " |";
      }
      out << "\n";
    }
    out << "\n";
  }
  out << "## Environments\n\n";
  out << "| | Operating System | Kernel Version | Architecture | Compiler |\n";
  out << "| - | - | - | - | - |\n";
  for (auto& environment : config.Environments) {
    EnvironmentInfo env_info = ParseEnvironmentInfo(environment.InfoPath);
    out << "| **" << environment.Description << "** | " << env_info.OS << " | "
        << env_info.KernelVersion << " | " << env_info.Architecture << " | "
        << env_info.Compiler << " |\n";
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    puts("Usage: report_generator <config file path>");
    return 0;
  }
  try {
    GenerateReport(argv[1]);
  } catch (const std::exception& e) {
    fprintf(stderr, "An error occurred: %s\n", e.what());
    return 1;
  }
  return 0;
}
