#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include"bigint.hpp"

#define ADD "+"
#define SUB "-"
#define MUL "*"
#define DIV "/"
#define MOD "%"

void test(const std::string &lhs, const std::string op, const std::string &rhs, const std::string &ans, int &success, int &failed)
{
  auto a = BigInt(lhs);
  auto b = BigInt(rhs);
  BigInt c;

  if (op == ADD)
    c = a + b;
  else if (op == SUB)
    c = a - b;
  else if (op == MUL)
    c = a * b;
  else if (op == DIV)
    c = a / b;
  else if (op == MOD)
    c = a % b;
  else
  {
    std::cerr << "Unknown operator!: " << op << std::endl;
    return;
  }

  bool succ = c.to_string() == ans;
  success += succ?1:0;
  failed += succ?0:1;

  if (succ) return;
  std::cout << "TEST:" << std::endl;
  std::cout << a.to_string() << " " + op + " " << b.to_string() << std::endl;
  std::cout << "= " << c.to_string() << std::endl;
  std::cout << "# " << ans << std::endl;
  std::cout << (succ?"SUCCESS":"FAIL!") << std::endl;
  std::cout << std::endl;
}

std::vector<std::string> split(const std::string &str, char sep)
{
    std::vector<std::string> v;
    std::stringstream ss(str);
    std::string buffer;
    while(std::getline(ss, buffer, sep))
    {
        v.push_back(buffer);
    }
    return v;
}

int main()
{
  int success = 0;
  int failed = 0;

  std::ifstream ifs("tests.txt");
  if (ifs.fail())
  {
    std::cerr << "Can't load tests!" << std::endl;
    return -1;
  }

  std::string line;
  int i = 0;
  while (std::getline(ifs, line))
  {
    std::cout << "\rTEST: " << ++i;

    auto v = split(line, '\t');
    if (v.size() != 4) break;

    test(v[0], v[1], v[2], v[3], success, failed);
  }

  std::cout << std::endl << "SUCCESS/FAILED: " << success << "/" << failed << std::endl;

  return 0;
}
