#pragma once
#include <iostream>

#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

class IniP {
    std::wstring trim(std::wstring str) {
        std::size_t index = 0;
        while (index < str.size() && (str[index] == L' ' || str[index] == L'\t')) ++index;
        str = str.substr(index);

        index = str.size() - 1;
        while (index > 0 && (str[index] == L' ' || str[index] == L'\t')) --index;
        str = str.substr(0, index + 1);

        return str;
    }

    struct Node {
        std::wstring key;
        std::wstring value;
        std::vector<Node> children;
    };

    std::string m_file;
    std::vector<Node> m_tree;

    std::wstring& _get(bool create, std::vector<Node>& tree, std::wstring key) {
        auto it = std::find_if(begin(tree), end(tree), [&](Node& t) { return t.key == key; });
        if (it != tree.end()) {
            return it->value;
        } else if (create) {
            tree.push_back({key, L"", {} });
            return tree[tree.size() - 1].value;
        } else {
            throw std::runtime_error("Key not found");
        }
    }

    template<typename... Args>
    std::wstring& _get(bool create, std::vector<Node>& tree, std::wstring key, Args... rest) {
        auto it = std::find_if(begin(tree), end(tree), [&](Node& t) { return t.key == key; });
        if (it != tree.end()) {
            return _get(create, it->children, rest...);
        } else if (create) {
            tree.push_back({key, L"", {} });
            return _get(create, tree[tree.size() - 1].children, rest...);
        } else {
            throw std::runtime_error("Key not found");
        }
    }

    void write(std::wofstream& file, Node& node, std::wstring parent = L"", std::size_t depth = 0) {
        parent = parent + node.key;

        if (depth == 0) {
            file << L"[" + node.key + L"]\n";
            for (auto& x : node.children) { write(file, x, L"", depth + 1); }
            file << L"\n";
        } else if (!node.children.empty()) {
            file << parent << L"=";
            if (!node.value.empty()) file << node.value;
            file << L"\n";
            for (auto& x : node.children) { write(file, x, parent + L"/", depth + 1); }
        } else {
            file << parent << L"=" << node.value << L"\n";
        }
    }

    void readKey(Node& node, std::wstring key, std::wstring value) {
        auto it = key.find(L"/");
        if (it != std::string::npos) {
            auto nit = std::find_if(begin(node.children), end(node.children), [&](const Node& node) {
                return node.key == key.substr(0, it);
            });

            if (nit == node.children.end()) {
                node.children.push_back({ key.substr(0, it), L"", {} });
                nit = node.children.end() - 1;
            }

            readKey(*nit, key.substr(it + 1), value);
        } else { node.children.push_back({ key, value, {} }); }
    }

    void read(std::wifstream& file, std::vector<Node>& tree) {
        std::wstring line;

        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == L'#') { continue; }

            if (line[0] == L'[' && line[line.size() - 1] == L']') {
                tree.push_back({line.substr(1, line.size() - 2), L"", {}});
            } else if (tree.size() > 0) {
                auto iton = line.find(L"=");
                if (iton != std::string::npos) {
                    std::wstring key = line.substr(0, iton);
                    std::wstring value = line.substr(iton + 1);
                    readKey(tree[tree.size() - 1], key, value);
                }
            }
        }
    }

public:
    /**
     * @brief Sets a value in hierarchy
     *
     * @param newval The new value to be set
     * @param rest Traversial path to place
     */
    template<typename... Args>
    void set(std::wstring newval, Args... rest) {
        _get(true, m_tree, std::forward<Args>(rest)...) = newval;
    }

    /**
     * @brief Gets a value from hierarchy
     *
     * @param rest Path of std::wstrings to be traversed
     *
     * @return Value of key
     */
    template<typename... Args>
    std::wstring get(Args... rest) {
        return _get(false, m_tree, std::forward<Args>(rest)...);
    }

    /**
     * @brief Reads a file
     *
     * @param file Path to file
     *
     * @return true on succesfull read
     */
    bool read(std::string file = "") {
        m_file = file.empty() ? m_file : file;
        std::wifstream f(m_file);
        if (f.is_open()) {
            read(f, m_tree);
            f.close();
            return true;
        }
        return true;
    }

    /**
     * @brief Writes a file
     *
     * @param file Path to file
     *
     * @return true on succesfull write
     */
    bool write(std::string file = "") {
        m_file = file.empty() ? m_file : file;
        std::wofstream f(m_file);
        if (f.is_open()) {
            for (auto& x : m_tree) { write(f, x); }
            f.close();
            return true;
        }
        return true;
    }
};
