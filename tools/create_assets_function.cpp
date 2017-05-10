/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * This tool makes creating asset's flatbuffer(Account, Currency, ...) helper.
 * Example as follows:
 *
   std::vector<uint8_t> CreateCurrency(const std::string &currencyName,
                                      const std::string &domainName,
                                      const std::string &ledgerName,
                                      const std::string &description,
                                      const std::string &amount,
                                      uint8_t precision) {
    flatbuffers::FlatBufferBuilder fbb;
    auto currency = iroha::CreateCurrencyDirect(
        fbb, currencyName.c_str(), domainName.c_str(), ledgerName.c_str(),
        description.c_str(), amount.c_str(), precision);
    auto asset =
        iroha::CreateAsset(fbb, ::iroha::AnyAsset::Currency, currency.Union());
    fbb.Finish(asset);
    auto buf = fbb.GetBufferPointer();
    return {buf, buf + fbb.GetSize()};
 */


#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

namespace utils {

std::string toLowersWhileUpper(std::string const &s, size_t &i) {
  std::string ret;
  for (; i < s.size(); i++) {
    if (::isupper(s[i])) ret += ::tolower(s[i]);
  }
  return ret;
}

std::string getCharsWhileLower(std::string const &s, size_t &i) {
  std::string ret;
  for (; i < s.size(); i++) {
    if (::islower(s[i])) ret += s[i];
  }
  return ret;
}

std::string toSnake(std::string const &s) {
  assert(!s.emtpy());
  std::string ret;
  size_t i = 0;
  while (i < s.size()) {
    ret += toLowersWhileUpper(s, i);
    ret += getCharsWhileLower(s, i);
    if (i != s.size()) {
      ret += "_";
    }
  }
  return ret;
}

bool hasPrefix(std::string& const target, std::string const& s) {
  if (target.size() < s.size()) return false;
  size_t psize = s.size();
  for (size_t i = 0; i < psize; i++) {
    if (target[i] != s[i]) return false;
  }
  return true;
}

bool hasSuffix(std::string& const target, std::string const& s) {
  auto newT = target;
  auto newS = s;
  reverse(newT.begin(), newT.end());
  reverse(newS.begin(), newS.end());
  return hasPrefix(newT, newS);
}

}  // namespace utils

class Field {
public:
  Field() = default;
  Field(std::string const& key, std::string const& type,
  std::vector<std::string> const& attr = {})
    : key_(key), type_(type), attribute_(attr) {}

  std::string primitiveType() {
    if (type_ == "string") return "std::string";
    if (type_ == "int") return "int";
    if (type_ == "ubyte") return "uint8_t";
    if (type_ == "ushort") return "uint8_t";
    return "flatbuffers::Offset<" + type_ + ">";
  }

  std::string cppType() {
    if (hasPrefix(type, "[")) {
      assert(hasSuffix(type, "]"));
      return "std::vector<" + primitiveType() + ">";
    }
    return primitiveType();
  }

  std::string argumentize() {
    return cppType() + " " + constRefIfNeeded() + " " + key_;
  }

private:
  std::string key_;
  std::string type_;
  std::vector<std::string> attribute_;
};

class Output {
public:

  static Output& getInstance() const {
    static Output instance_;
    return instance_;
  }

  size_t indent() const {
    return indent_;
  }

  std::string indentString() const {
    return std::string(indent, ' ');
  }

  void doNest() {
    indent_ += 2;
  }

  void doUnNest() {
    indent_ -= 2;
    assert(indent_ >= 0);
  }

  void append(std::string const& s) {
    ret += indentString() + s + "\n";
  }

  std::string result() const {
    return result_;
  }

private:
  Output() {}
  size_t indent_;
  std::string result_;
};

class Table {
public:
  bool empty() {
    return tableName.empty();
  }

  /*
    flatbuffers::FlatBufferBuilder fbb;
    auto currency = iroha::CreateCurrencyDirect(
        fbb, currencyName.c_str(), domainName.c_str(), ledgerName.c_str(),
        description.c_str(), amount.c_str(), precision);
    auto asset =
        iroha::CreateAsset(fbb, ::iroha::AnyAsset::Currency, currency.Union());
    fbb.Finish(asset);
    auto buf = fbb.GetBufferPointer();
    return {buf, buf + fbb.GetSize()};
   */

  std::string output() {
    std::string ret;
    // TODO(motxx): Enclose all functions in one namespace
    append(ret, "namespace " + toSnake(tableName) + " {");
    nestIndent();
    {
      append(ret, returnType + " Create" + tableName + "(");
      nestIndent();
      for (size_t i = 0; i < fields.size(); i++) {
        append(ret, argumentize(fields[i])
                    + (i < fields.size() - 1 ? "," : ""));
      }
      unnestIndent();
      append(ret, ") {");
      nestIndent();
      {
        append(ret, "flatbuffers::FlatBufferBuilder fbb;");
        // TODO(motxx): Nested user-defined object (Offset<T>)
      }
      unnestIndent();
    }
    unnestIndent();
    append(ret, "}  // namespace " + toSnake(tableName));
    return ret;
  }

  static Table parseImpl(std::vector<std::string> const& tokens) {
    if (tokens.size() != 3) return Table();
    if (tokens[0] != "table") return Table();

    assert(tokens.size() == 3);
    assert(tokens[2] == "{");

    Table ret;
    ret.tableName_ = tokens[1];
    // ToDo(motxx): Parse fields.
    //ret.fields_;
    return ret;
  }

  static bool parse(std::vector<std::string> const& tokens) {
    auto res = parseImpl(tokens);
    if (!res.empty()) {
      // ToDo(motxx): It's only for debugging. Return output.
      std::cout << res.output() << std::endl;
      return true;
    }
    return false;
  }

  const char* returnType() const {
    return "std::vector<uint8_t>";
  }

private:
  std::string tableName_;
  std::vector<Field> fields_;
};

class Include {
public:
  static bool parse(std::vector<std::string> const& tokens) {
    if (tokens.size() != 2) return false;
    if (tokens[0] != "include") return false;

    if (recursive) {
      // ToDo(motxx): recursive parse
    }
    std::cout << "include is ignored.\n";
    return true;
  }

private:

};

class Parser {
public:

  Parser() = default;

  Parser(std::string const& fbsPath, bool recursive = false)
    : recursive(recursive) {
    ifs.open(fbsPath);
    if (ifs.fail()) {
      std::cerr << "Failed to load '" << fbsname << "'\n";
      exit(1);
    }
    parse();
  }

  std::vector<std::string> tokenize(std::string const& line) {
    std::vector<std::string> ret;
    std::stringstream ss(line);
    std::string token;
    while (ss >> token) {
      if (hasPrefix(token, "//")) return ret;
      ret.push_back(token);
    }
    return ret;
  }

  void parse() {
    std::string line;
    while (std::getline(ifs, line)) {
      if (line.empty()) continue;
      auto tokens = tokenize(line);

      if (Table::parse(tokens)) continue;
      if (Include::parse(tokens)) continue;

      std::cout << "Unrecognized tokens: ";
      dump(tokens);
    }
  }

  void dump(std::vector<std::string> const& tokens) {
    for (auto const& e: tokens) {
      std::cout << e << " ";
    }
    std::cout << std::endl;
  }

private:
  std::ifstream ifs;
  bool recursive; // read include files
};

int main() {

}
