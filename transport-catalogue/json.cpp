#include "json.h"

using namespace std;
namespace json {

    Node LoadNode(istream& input);

    std::string LoadLiteral(std::istream& input) {
        std::string str;

        while (std::isalpha(input.peek())) {
            str.push_back(static_cast<char>(input.get()));
        }
        return str;
    }

    Node LoadArray(std::istream& input) {
        std::vector<Node> array;

        for (char ch; input >> ch && ch != ']';) {
            if (ch != ',') {
                input.putback(ch);
            }

            array.push_back(LoadNode(input));
        }

        if (!input) {
            throw ParsingError("unable to parse array"s);
        }

        return Node(array);
    }

    Node LoadNull(std::istream& input) {
        if (auto literal = LoadLiteral(input); literal == "null"sv) {
            return Node(nullptr);
        }
        else {
            throw ParsingError("unable to parse '"s + literal + "' as null"s);
        }
    }

    Node LoadBool(std::istream& input) {
        const auto str = LoadLiteral(input);

        if (str == "true"sv) {
            return Node(true);
        }
        else if (str == "false"sv) {
            return Node(false);
        }
        else {
            throw ParsingError("unable to parse '"s + str + "' as bool"s);
        }
    }

    Node LoadNumber(std::istream& input) {
        std::string number;

        auto read_char = [&number, &input] {
            number += static_cast<char>(input.get());

            if (!input) {
                throw ParsingError("unable to read number"s);
            }
        };

        auto read_digits = [&input, read_char] {

            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            else {
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            }
        };

        if (input.peek() == '-') {
            read_char();
        }

        if (input.peek() == '0') {
            read_char();
        }
        else {
            read_digits();
        }

        bool is_int = true;
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();

            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }

            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                try {
                    return Node(std::stoi(number));

                }
                catch (...) {}
            }
            return Node(std::stod(number));

        }
        catch (...) {
            throw ParsingError("unable to convert "s + number + " to number"s);
        }
    }

    Node LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            }
            else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return Node(s);
    }

    Node LoadDict(std::istream& input) {
        Dict dictionary;

        for (char ch; input >> ch && ch != '}';) {

            if (ch == '"') {
                std::string key = LoadString(input).AsString();

                if (input >> ch && ch == ':') {

                    if (dictionary.find(key) != dictionary.end()) {
                        throw ParsingError("duplicate key '"s + key + "'found");
                    }

                    dictionary.emplace(std::move(key), LoadNode(input));

                }
                else {
                    throw ParsingError(": expected. but '"s + ch + "' found"s);
                }

            }
            else if (ch != ',') {
                throw ParsingError("',' expected. but '"s + ch + "' found"s);
            }
        }

        if (!input) {
            throw ParsingError("unable to parse dictionary"s);
        }
        else {
            return Node(dictionary);
        }

    }

    Node LoadNode(std::istream& input) {
        char ch;

        if (!(input >> ch)) {
            throw ParsingError(""s);
        }
        else {
            switch (ch) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't': case 'f':
                input.putback(ch);
                return LoadBool(input);
            case 'n':
                input.putback(ch);
                return LoadNull(input);
            default:
                input.putback(ch);
                return LoadNumber(input);
            }
        }
    }

    // методы, возвращающие значение в Node //
    bool Node::AsBool() const {
        using namespace std::literals;

        if (!IsBool())
            throw std::logic_error("value is not boolean"s);
        else
            return std::get<bool>(value_);
    }

    int Node::AsInt() const {
        using namespace std::literals;

        if (!IsInt())
            throw std::logic_error("value is not integer"s);
        else
            return std::get<int>(value_);
    }

    double Node::AsDouble() const {
        using namespace std::literals;

        if (!IsDouble())
            throw std::logic_error("value is not a double"s);
        else if (IsPureDouble())
            return std::get<double>(value_);
        else
            return AsInt();
    }

    const string& Node::AsString() const {
        using namespace std::literals;

        if (!IsString())
            throw std::logic_error("value is not a string"s);
        else
            return std::get<std::string>(value_);
    }

    const Array& Node::AsArray() const {
        using namespace std::literals;

        if (!IsArray())
            throw std::logic_error("value is not an array");
        else
            return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        using namespace std::literals;
        if (!IsMap())
            throw std::logic_error("value is not a dictionary");
        else
            return std::get<Dict>(value_);
    }

    // методы, проверяющие тип данных в Node //
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    Document::Document(Node root) : root_(std::move(root)) {}
    const Node& Document::GetRoot() const { return root_; }
    Document Load(istream& input) { return Document{ LoadNode(input) }; }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        [[nodiscard]] PrintContext Indented() const {
            return { out,
                    indent_step,
                    indent + indent_step };
        }
    };

    void PrintNode(const Node& node, const PrintContext& context);


    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');

        for (const char ch : value) {
            switch (ch) {
            case '\r':
                out << R"(\r)";
                break;
            case '\n':
                out << R"(\n)";
                break;
            case '\t':
                out << R"(\t)";
                break;
            case '"':
                out << R"(\")";
                break;
            case '\\':
                out << R"(\\)";
                break;
            default:
                out.put(ch);
                break;
            }
        }

        out.put('"');
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& context) {
        context.out << value;
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& context) {
        PrintString(value, context.out);
    }

    void PrintValue(const std::nullptr_t&, const PrintContext& context) {
        context.out << "null"s;
    }

    void PrintValue(bool value, const PrintContext& context) {
        context.out << std::boolalpha << value;
    }

    [[maybe_unused]] void PrintValue(Array nodes, const PrintContext& context) {
        std::ostream& out = context.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_context = context.Indented();

        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }

            inner_context.PrintIndent();
            PrintNode(node, inner_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put(']');
    }

    [[maybe_unused]] void PrintValue(Dict nodes, const PrintContext& context) {
        std::ostream& out = context.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_context = context.Indented();

        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }

            inner_context.PrintIndent();
            PrintString(key, context.out);
            out << ": "sv;
            PrintNode(node, inner_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& context) {
        std::visit([&context](const auto& value) {
            PrintValue(value, context);
            }, node.GetValue());
    }

    void Print(const Document& document, std::ostream& output) {
        PrintNode(document.GetRoot(), PrintContext{ output });
    }

}//end namespace json