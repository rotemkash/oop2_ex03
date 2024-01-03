#pragma once

#include <iosfwd>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// NEW
#include <filesystem>
#include <fstream>
#include <istream>
#include <system_error>

#include "Exceptions.h"

class Operation;

class FunctionCalculator {
 public:
  FunctionCalculator(std::istream& istr, std::ostream& ostr);
  void run();

 private:
  void eval();
  void substr();
  void mul();
  void del();
  void help();
  void read();
  void resize();
  void exit();

  bool checkArgsCount(std::string const& input, int argsC);

  void getSizeFromUser();
  bool continueReadingFile();

  template <typename FuncType>
  void binaryFunc() {
    auto line = std::string();
    std::getline(m_istr, line);
    std::stringstream strStream(line);
    try {
      if (auto f0 = readOperationIndex(strStream),
          f1 = readOperationIndex(strStream);
          f0 && f1) {
        if (m_operations.size() >= m_capacity) {
          throw std::overflow_error("Max function number reached\n");
        }
        m_operations.push_back(
            std::make_shared<FuncType>(m_operations[*f0], m_operations[*f1]));
      }
    } catch (...) {
      throw;
    }
  }

  void printOperations() const;

  enum class Action {
    Invalid,
    Eval,
    SubStr,
    Add,
    Mul,
    Comp,
    Del,
    Read,
    Resize,
    Help,
    Exit,
  };

  struct ActionDetails {
    std::string command;
    std::string description;
    Action action;
  };

  using ActionMap = std::vector<ActionDetails>;
  using OperationList = std::vector<std::shared_ptr<Operation>>;

  const ActionMap m_actions;
  OperationList m_operations;
  bool m_running = true;
  std::istream& m_istr;
  std::ostream& m_ostr;

  std::optional<int> readOperationIndex(std::stringstream& strStream) const;
  Action readAction() const;
  void runAction(Action action);

  ActionMap createActions() const;
  OperationList createOperations() const;

  bool m_readingFile = false;

  int m_capacity = 3;
  const int m_minSize = 3, m_maxSize = 100;
  const int m_maxStrLen = 20;
};
