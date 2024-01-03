
#include "FunctionCalculator.h"

#include <algorithm>
#include <iostream>

#include "Add.h"
#include "Comp.h"
#include "Identity.h"
#include "Mul.h"
#include "Reverse.h"
#include "SubStr.h"
#include "Swap.h"

FunctionCalculator::FunctionCalculator(std::istream& istr, std::ostream& ostr)
    : m_actions(createActions()),
      m_operations(createOperations()),
      m_istr(istr),
      m_ostr(ostr) {}

/*
 * This Function runs the program
 */
void FunctionCalculator::run() {
  // if not reading from a file ask the user to enter the size of the operations
  // vector
  if (!m_readingFile) {
    m_ostr << "Please Enter the max number (between 3-100) of functions you "
              "want to handle\n";
    while (true) {
      try {
        m_istr.putback(' ');
        getSizeFromUser();
        break;
      } catch (InputFailure& e) {
        m_istr.clear();
        m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        m_ostr << e.what();
      } catch (const std::exception& e) {
        m_ostr << e.what();
      }
    }
  }

  do {
    m_ostr << '\n';
    printOperations();
    m_ostr << "Enter command ('help' for the list of available commands): ";

    try {
      // if we are reading from a file print the command to the screen
      if (m_readingFile) {
        if (m_istr.eof()) {
          throw std::exception("End Of File\n");
        }
        int tmp = m_istr.tellg();
        auto tmpCommand = std::string();

        std::getline(m_istr, tmpCommand);
        m_ostr << tmpCommand << std::endl;
        m_istr.seekg(tmp);
      }

      const auto action = readAction();
      runAction(action);

    } catch (InvalidAction& e) {
      m_ostr << e.what();
      m_ostr.clear();
      m_istr.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (m_readingFile) {
        throw;
      }
    } catch (const std::exception& e) {
      m_ostr << e.what() << std::endl;
      if (m_readingFile) {
        throw;
      }
    } catch (...) {
      if (m_readingFile) {
        throw;
      }
    }

    if (m_istr.eof()) {
      m_running = false;
    }
  } while (m_running);
}

/*
 * This function recieves a string(the user's input) and a the number of
 * arguments that should be in the command, it checks if the arguments count is
 * correct and returns accordingly
 */
bool FunctionCalculator::checkArgsCount(std::string const& input, int argsC) {
  auto tmp = std::string();
  std::vector<std::string> splitCommand;

  // string stream so it's easier to split the line
  std::stringstream strStream(input);
  strStream.ignore();

  // split the line into a vector
  while (std::getline(strStream, tmp, ' ')) {
    splitCommand.push_back(tmp);
  }
  return splitCommand.size() == argsC;
}

/*
 * This function asks the uer to enter the size of the operations
 * vector he wants
 */
void FunctionCalculator::getSizeFromUser() {
  int tmp_size = 0;

  auto line = std::string();
  std::getline(m_istr, line);
  std::stringstream strStream(line);
  // check if the number of arguemnts is 1
  if (!checkArgsCount(line, 1)) {
    throw InvalidArguemntCount();
  }

  if (!(strStream >> tmp_size)) {
    throw InputFailure();
  }

  if (tmp_size < m_minSize || tmp_size > m_maxSize) {
    throw std::out_of_range("Number is out of range, try again!\n");
  }

  m_capacity = tmp_size;
}

/*
 * This function is called when an invalid command is met while reading
 * from a file. It asks the user whether he wants to continue to read
 * from the file or exit and continue to get input normally
 */
bool FunctionCalculator::continueReadingFile() {
  auto answer = std::string();
  m_ostr
      << "There was a problem in the file, reason is listed above this line.\n";
  m_ostr << "Do you want to continue readign the rest of the file? yes/no\n";

  while (true) {
    try {
      if (!(m_istr >> answer)) {
        throw InputFailure();
      }

      if (answer == "yes") {
        return true;
      } else if (answer == "no") {
        return false;
      } else {
        throw InputFailure();
      }
    } catch (const std::exception& e) {
      m_ostr << e.what();
      continue;
    }
  }
}

/*
 * This function evaluates the given string according to the operation given
 */
void FunctionCalculator::eval() {
  auto line = std::string();
  std::getline(m_istr, line);
  std::stringstream strStream(line);
  // check if the number of arguemnts is 2
  if (!checkArgsCount(line, 2)) {
    throw InvalidArguemntCount();
  }

  if (auto index = readOperationIndex(strStream); index) {
    const auto& operation = m_operations[*index];

    auto input = std::string();
    if (!(strStream >> input)) {
      throw InputFailure();
    }
    if (input.length() > m_maxStrLen) {
      throw std::length_error("The string length is too big\n");
    }
    operation->print(m_ostr, input);
    m_ostr << " = " << operation->compute(input) << '\n';
  }
}

/* asks the user for values and creates a substring function*/
void FunctionCalculator::substr() {
  auto start_index = 0, chars_length = 0;
  auto line = std::string();
  // m_istr.ignore();
  std::getline(m_istr, line);  // get the whole line from user
  // check if the number of arguemnts is 2
  if (!checkArgsCount(line, 2)) {
    throw InvalidArguemntCount();
  }
  // copy the line to string stream to read from it easily
  std::stringstream strStream(line);

  if (!(strStream >> start_index)) {
    throw InputFailure();
  }

  if (start_index < 0) {
    throw std::invalid_argument("Cannot use SubStr with negative value");
  }

  if (!(strStream >> chars_length)) {
    throw InputFailure();
  }

  if (m_operations.size() >= m_capacity) {
    throw std::overflow_error("Max function number reached\n");
  }
  m_operations.push_back(std::make_shared<SubStr>(start_index, chars_length));
}

/*creates a mul function with the matching function and number*/
void FunctionCalculator::mul() {
  auto n = 0;

  auto line = std::string();
  std::getline(m_istr, line);
  // check if the number of arguemnts is 2
  if (!checkArgsCount(line, 2)) {
    throw InvalidArguemntCount();
  }
  std::stringstream strStream(line);

  if (!(strStream >> n)) {
    throw InputFailure();
  }

  if (n < 0) {
    throw std::invalid_argument("cannot mul negative value");
  }

  if (auto i = readOperationIndex(strStream); i) {
    if (m_operations.size() >= m_capacity) {
      throw std::overflow_error("Max function number reached\n");
    }
    m_operations.push_back(std::make_shared<Mul>(n, m_operations[*i]));
  }
}

/*asks the user for a number and deletes the funciton with that number*/
void FunctionCalculator::del() {
  auto line = std::string();
  std::getline(m_istr, line);
  // check if the number of arguemnts is 1
  if (!checkArgsCount(line, 1)) {
    throw InvalidArguemntCount();
  }
  std::stringstream strStream(line);

  if (auto i = readOperationIndex(strStream); i) {
    if (m_operations.size() <= m_minSize) {
      throw std::overflow_error(
          "Min function number reached, can't delete more function\n");
    }

    m_operations.erase(m_operations.begin() + *i);
  }
}

void FunctionCalculator::help() {
  auto line = std::string();
  std::getline(m_istr, line);
  std::stringstream strStream(line);
  // check if the number of arguemnts is 0
  if (!checkArgsCount(line, 0)) {
    throw InvalidArguemntCount();
  }

  m_ostr << "The available commands are:\n";
  for (const auto& action : m_actions) {
    m_ostr << "* " << action.command << action.description << '\n';
  }
  m_ostr << '\n';
}

/*Reads the commands from a file given by the user*/
void FunctionCalculator::read() {
  std::string path;
  m_istr.ignore();
  std::getline(m_istr, path);
  std::cout << path << "\n";
  std::ifstream opFile;

  opFile.open(path);
  if (!opFile) {
    throw std::runtime_error("failed to read file");
  }

  auto fileCalc = FunctionCalculator(opFile, std::cout);
  // copy the parameters from the current calcualtor
  fileCalc.m_readingFile = true;
  fileCalc.m_operations = this->m_operations;
  fileCalc.m_capacity = this->m_capacity;

  while (fileCalc.m_running) {
    try {
      fileCalc.run();
    } catch (...) {
      fileCalc.m_running = continueReadingFile();
    }
  }

  // update the calcualtor parameters according to what was done in the file
  this->m_operations = fileCalc.m_operations;
  this->m_capacity = fileCalc.m_capacity;
}

/* Resizes the operations vector */
void FunctionCalculator::resize() {
  int oldSize = m_capacity;
  try {
    //m_istr.ignore();
    getSizeFromUser();

    if (m_capacity < m_operations.size()) {
      auto answer = std::string();
      m_ostr
          << "The new size is smaller than the current vector. Do you want to "
             "delete the excces operations or do you want to cancel the "
             "resize?\n";
      m_ostr << "delete / cancel\n";

      while (true) {
        if (!(m_istr >> answer)) {
          m_ostr << "Failed to read, try again\n";
          continue;
        }

        if (answer == "delete") {
          m_operations.resize(m_capacity);
          break;
        } else if (answer == "cancel") {
          m_capacity = oldSize;
          break;
        } else {
          m_ostr << "Invalid input, try again\n";
        }
      }
    }

  } catch (const std::exception& e) {
    m_ostr << e.what();
  }
}

/* exits the program */
void FunctionCalculator::exit() {
  auto line = std::string();
  std::getline(m_istr, line);
  std::stringstream strStream(line);
  // check if the number of arguemnts is 0
  if (!checkArgsCount(line, 0)) {
    throw InvalidArguemntCount();
  }
  m_ostr << "Goodbye!\n";
  m_running = false;
}

/* Prints the avialble operations */
void FunctionCalculator::printOperations() const {
  m_ostr << "List of available string operations:\n";
  m_ostr << "### Max Capacity = " << m_capacity << " ###\n";
  for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i) {
    m_ostr << i << ".\t";
    m_operations[i]->print(m_ostr, true);
    m_ostr << '\n';
  }
  m_ostr << '\n';
}

/* gets an int and tries */
std::optional<int> FunctionCalculator::readOperationIndex(
    std::stringstream& strStream) const {
  auto i = 0;

  if (!(strStream >> i)) {
    throw InputFailure();
  }
  if (i >= m_operations.size() || i < 0) {
    throw InvalidOperation();
    m_ostr << "Operation #" << i << " doesn't exist\n";
    return {};
  }
  return i;
}

/* rads the action the user asked for and tries to find it in the vector*/
FunctionCalculator::Action FunctionCalculator::readAction() const {
  auto action = std::string();
  if (!(m_istr >> action)) {
    throw InputFailure();
  }

  const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
  if (i != m_actions.end()) {
    return i->action;
  }

  throw InvalidAction();
}

void FunctionCalculator::runAction(Action action) {
  switch (action) {
    default:
      m_ostr << "Unknown enum entry used!\n";
      break;

    case Action::Invalid:
      m_ostr << "Command not found\n";
      break;

    case Action::Eval:
      eval();
      break;
    case Action::SubStr:
      substr();
      break;
    case Action::Add:
      binaryFunc<Add>();
      break;
    case Action::Mul:
      mul();
      break;
    case Action::Comp:
      binaryFunc<Comp>();
      break;
    case Action::Del:
      del();
      break;
    case Action::Help:
      help();
      break;
    case Action::Read:
      read();
      break;
    case Action::Resize:
      resize();
      break;
    case Action::Exit:
      exit();
      break;
  }
}

FunctionCalculator::ActionMap FunctionCalculator::createActions() const {
  return ActionMap{
      {"eval",
       "(uate) num x - compute the result of function #num on the "
       "following x string",
       Action::Eval},
      {"substr",
       " x y - creates a substring starting from index x with the length of y "
       "characters "
       "following x string",
       Action::SubStr},
      {"add",
       "(on) num1 num2 - Creates an operation that is the concatenation of "
       "operation #num1 and operation #num2",
       Action::Add},
      {"mul",
       "(tiply) n num - Creates an operation that is the "
       "multiply n of operation #num",
       Action::Mul},
      {"comp",
       "(osite) num1 num2 - creates an operation that is the composition "
       "of operation #num1 and operation #num2",
       Action::Comp},
      {"del", "(ete) num - delete operation #num from the operation list",
       Action::Del},
      {"read", "filePath - read commands from file", Action::Read},
      {"resize", "num - resize the operations vector", Action::Resize},
      {"help", " - print this command list", Action::Help},
      {"exit", " - exit the program", Action::Exit}};
}

FunctionCalculator::OperationList FunctionCalculator::createOperations() const {
  return OperationList{std::make_shared<Identity>(), std::make_shared<Swap>(),
                       std::make_shared<Reverse>()

  };
}
