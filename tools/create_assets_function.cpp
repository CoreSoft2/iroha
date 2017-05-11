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
#include <memory>
#include <assert.h>

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

bool hasPrefix(std::string& const target, std::string const& prefix) {
  if (target.size() < prefix.size()) return false;
  size_t psize = prefix.size();
  for (size_t i = 0; i < psize; i++) {
    if (target[i] != prefix[i]) return false;
  }
  return true;
}

bool hasSuffix(std::string& const target, std::string const& prefix) {
  auto newTarget = target;
  auto newPrefix = prefix;
  reverse(newTarget.begin(), newTarget.end());
  reverse(newPrefix.begin(), newPrefix.end());
  return hasPrefix(newTarget, newPrefix);
}

}  // namespace utils

namespace lexer {

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

std::vector<std::string> lexLine(std::ifstream& ifs) {
  std::string line;
  std::getline(ifs, line);
  assert(!ifs.fail());
  return lexer::tokenize(line);
}

}  // namespace lexer


class Input {
public:

  static Input& getInstance() {
    static Input instance_;
    return instance_;
  }

  static void readSchema(std::string const& fbsPath) {
    ifs_ = std::ifstream(fbsPath);
  }

  std::vector<std::string> nextTokens() {
    return lexer::lexLine(ifs_);
  }

  bool eof() {
    return ifs_.eof() || ifs_.fail();
  }

private:
  Input() {}
  std::ifstream ifs_;
};

class Output {
public:

  constexpr int IndentSize = 2;

  static Output& getInstance() {
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
    indent_ += IndentSize;
  }

  void doUnNest() {
    indent_ -= IndentSize;
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

class IOStream {
protected:
  Input& in = Input::getInstance();
  Output& out = Output::getInstance();
};

template <typename> class ObjectFactory;

template <typename T, typename... Args>
class ObjectFactory {
public:
  virtual ~ObjectFactory() {}
  static T parse(Args&&... args) {
    return T::parseInternal(std::forward<Args>(args)...);
  }
};

using Tokens = std::vector<std::string>;
using TokensRef = Tokens const&

class Field final : public ObjectFactory<Field(TokensRef)>, public IOStream {
public:
  virtual ~Field() {}

  static std::shared_ptr<Field> parseInternal(TokensRef tokens) {

    assert(tokens.size() >= 4);
    auto const& variable = tokens[0];
    assert(tokens[1] == ":");
    auto const& type = tokens[2];

    if (tokens[3] == "(") {
      auto args = parseArguments();
    } else {
      assert(tokens[3] == ";");
    }

    return std::make_shared<Field>();
  }

private:
  Field() {}
  std::string key_;
  std::string type_;
  std::vector<std::string> attribute_;
};

class FieldSet final : public ObjectFactory<FieldSet()>, public IOStream {
public:
  static FieldSet parse() {
    FieldSet ret;
    assert(!in.eof());
    for (;;) {
      auto tokens = in.nextTokens();
      if (tokens.empty()) continue;
      if (tokens[0] == "}") break;
      ret.fieldSet_.push_back(ObjectFactory<Field>::parse(tokens));
    }
    return ret;
  }

private:
  FieldSet() {}
  std::vector<std::shared_ptr<Field>> fieldSet_;
};

class Table final : public ObjectFactory<Table(TokensRef)> {
public:
  bool null() {
    return tableName.empty();
  }

  explicit operator bool() const {
    return !null();
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

  static Table parseInternal(TokensRef tokens) {
    if (tokens.size() != 3) return Table();
    if (tokens[0] != "table") return Table();

    assert(tokens.size() == 3);
    assert(tokens[2] == "{");

    out.append(returnType + " Create" + tableName_ + "(");
    out.doNest();
    for (size_t i = 0; i < fields.size(); i++) {
      Output.getInstance().append(argumentize(fields[i])
                                  + (i < fields.size() - 1 ? "," : ""));
    }
    out.doUnNest();
    out.append(") {");
    out.doNest();
    {
      out.append("flatbuffers::FlatBufferBuilder fbb;");
      out.append("auto o = iroha::Create" + tableName_ + "Direct(");
      out.doNest();
      {
        auto fieldTokens = lex<Field>();
        auto field = ObjectFactory<Field>::parse(fieldTokens);

        out.append("fbb, ");
      }
      out.doUnNest();
      // TODO(motxx): Nested user-defined object (Offset<T>)
    }
    out.doUnnest();
    return ret;
  }

  std::string returnType() const {
    return "std::vector<uint8_t>";
  }

private:
  Table() {}
  std::string tableName_;
  std::shared_ptr<FieldSet> fieldSet_;
};

class Include : public ObjectFactory<Include>, public IOStream {
public:
  static Include parse(std::vector<std::string> const& tokens, bool recursive) {
    if (tokens.size() != 2) return false;
    if (tokens[0] != "include") return false;

    if (recursive) {
      // ToDo(motxx): recursive parse
    }
    std::cout << "include is ignored.\n";
    return true;
  }

private:
  Include() {}
};

class Schema : public ObjectFactory<Schema>, public IOStream {
public:

  static Schema parseInternal() {

    // ToDo(motxx): Move appropriate func
    out.append("namespace " + toSnake(tableName) + " {");
    nestIndent();

    while (true) {
      auto tokens = lexer::lexLine(ifs);
      if (tokens.empty()) break;

      if (Table::parse(tokens)) continue;
      if (Include::parse(tokens, recursive)) continue;

      std::cout << "Unrecognized tokens: ";
      dump(tokens);
    }
    unnestIndent();
    out.append("}  // namespace " + toSnake(tableName));
  }

  void dump(std::vector<std::string> const& tokens) {
    for (auto const& e: tokens) {
      std::cout << e << " ";
    }
    std::cout << std::endl;
  }

private:
  Schema() {}
  bool recursive; // read include files
};

class CodeGenerator {
public:
  virtual std::string generate() = 0;
  virtual ~CodeGenerator() {}
};

class FieldSetGenerator : public CodeGenerator {
public:
  FieldSetGenerator(FieldSet const& fldSet)
    : fieldSet_(fldSet) {}

  virtual ~FieldGenerator() {}


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

  std::string constRefIfNeeded() const {
    if (type_ == "string") return "const&";
    return "";
  }

  std::string argumentize() {
    return cppType() + " " + constRefIfNeeded() + " " + key_;
  }

  virtual std::string generate() {

  }

private:
  FieldSet fieldSet_;
};

class TableGenerator : public CodeGenerator {
public:
  TableGenerator(Table const& tbl)
    : table_(tbl) {}

  virtual ~TableGenerator() {}

  virtual std::string generate() {

  }

private:
  Table table_;
};

class SchemaGenerator : public CodeGenerator {
public:
  SchemaGenerator(Schema const& sch)
    : schema_(sch) {}

  virtual ~SchemaGenerator() {}

  virtual std::string generate() {

  }

private:
  Schema schema_;
};

int main() {

}
