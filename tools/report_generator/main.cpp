// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

struct EnvironmentInfo {
  std::string Description;
  std::string Path;

  friend void to_json(nlohmann::json& j, const EnvironmentInfo& e) {
    j = nlohmann::json{
        {"Description", e.Description},
        {"Path", e.Path}
    };
  }

  friend void from_json(const nlohmann::json& j, EnvironmentInfo& e) {
    j.at("Description").get_to(e.Description);
    j.at("Path").get_to(e.Path);
  }
};

struct MetricInfo {
  std::string Name;
  std::string TargetBenchmarkName;
  std::string BaselineBenchmarkName;

  friend void to_json(nlohmann::json& j, const MetricInfo& m) {
    j = nlohmann::json{
        {"Name", m.Name},
        {"TargetBenchmarkName", m.TargetBenchmarkName},
        {"BaselineBenchmarkName", m.BaselineBenchmarkName}
    };
  }

  friend void from_json(const nlohmann::json& j, MetricInfo& m) {
    j.at("Name").get_to(m.Name);
    j.at("TargetBenchmarkName").get_to(m.TargetBenchmarkName);
    j.at("BaselineBenchmarkName").get_to(m.BaselineBenchmarkName);
  }
};

struct ReportConfig {
  std::string TargetName;
  double YellowIndicatorThreshold;
  std::string OutputPath;
  std::vector<EnvironmentInfo> Environments;
  std::vector<MetricInfo> Metrics;

  friend void to_json(nlohmann::json& j, const ReportConfig& rc) {
    j = nlohmann::json{
        {"TargetName", rc.TargetName},
        {"YellowIndicatorThreshold", rc.YellowIndicatorThreshold},
        {"OutputPath", rc.OutputPath},
        {"Environments", rc.Environments},
        {"Metrics", rc.Metrics}
    };
  }

  friend void from_json(const nlohmann::json& j, ReportConfig& rc) {
    j.at("TargetName").get_to(rc.TargetName);
    j.at("YellowIndicatorThreshold").get_to(rc.YellowIndicatorThreshold);
    j.at("OutputPath").get_to(rc.OutputPath);
    j.at("Environments").get_to(rc.Environments);
    j.at("Metrics").get_to(rc.Metrics);
  }
};

const std::string_view MedianSuffix = "_median";

std::unordered_map<std::string, double> Parse(const std::filesystem::path& file) {
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
    benchmarks.push_back(Parse(environment.Path));
  }
  std::ofstream out;
  out.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  out.open(config.OutputPath, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
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
  for (auto& metric : config.Metrics) {
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
        out << "\xf0\x9f\x9f\xa1";  // Yellow circle
      } else if (is_negative) {
        out << "\xf0\x9f\x94\xb4";  // Red circle
      } else {
        out << "\xf0\x9f\x9f\xa2";  // Green circle
      }
      auto rate_str = std::format("{:.1f}", rate);
      std::string message;
      if (rate_str == "0.0") {
        out << config.TargetName << " has similar performance";
      } else {
        out << config.TargetName << " is about **" << rate_str << "% " << (is_negative ? "slower" : "faster") << "**";
      }
      out << " |";
    }
    out << "\n";
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
