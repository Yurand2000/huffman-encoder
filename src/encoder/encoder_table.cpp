#include <algorithm>
#include <unordered_map>
#include <memory>

#include "encoder_table.h"
#include "../utils.h"

namespace huffman::encoder::detail
{
    inline std::vector<byte> serialize_character(const encoderTable& table, char character)
    {
        auto encoding = table.get(character);

        auto serialized = std::vector<byte>();
        serialized.resize(2 + encoding.code.size());
        serialized[0] = static_cast<byte>(character);
        serialized[1] = encoding.bits;
        serialized.insert(
            std::end(serialized),
            std::begin(encoding.code),
            std::end(encoding.code)
        );

        return serialized;
    }
    
    inline encodedCharacter deserialize_character(std::vector<byte>::const_iterator& bytes, char& character)
    {
        character = static_cast<char>(*bytes);

        auto encoding = encodedCharacter();
        encoding.bits = *(++bytes);

        auto number_of_bytes = positive_div_ceil(encoding.bits, static_cast<byte>(8));
        encoding.code.reserve(number_of_bytes);

        for (size_t i = 0; i < number_of_bytes; i++) {
            encoding.code.push_back( *(++bytes) );
        }

        ++bytes;
        return encoding;
    }

    struct encoderTree {
        char character;
        int frequency;
        std::unique_ptr<encoderTree> left, right;

        friend bool operator<(const encoderTree& lhs, const encoderTree& rhs);
    };

    std::unordered_map<char, int> extract_frequencies(const std::string& text) {
        auto frequencies = std::unordered_map<char, int>();

        for (auto character: text) {
            if (frequencies.find(character) != frequencies.end()) {
                frequencies[character] += 1;
            } else {
                frequencies.emplace(character, 1);
            }
        }

        return frequencies;
    }

    std::unique_ptr<encoderTree> build_encoder_tree(const std::unordered_map<char, int>& frequencies) {
        auto heap = std::vector<std::unique_ptr<encoderTree>>();
        heap.reserve(frequencies.size());

        for (auto const& [character, frequency] : frequencies) {
            auto node = std::make_unique<encoderTree>();
            node->character = character;
            node->frequency = frequency;
            heap.emplace_back(std::move(node));
        }

        std::make_heap(heap.begin(), heap.end(), std::greater<>{});

        while (!heap.empty()) {
            std::pop_heap(heap.begin(), heap.end(), std::greater<>{});
            auto smallest_1 = std::move(heap.back()); heap.pop_back();

            if (heap.empty()) {
                return smallest_1;
            } else {
                std::pop_heap(heap.begin(), heap.end(), std::greater<>{});
                auto smallest_2 = std::move(heap.back()); heap.pop_back();

                auto node = std::make_unique<encoderTree>();
                node->character = '\0';
                node->frequency = smallest_1->frequency + smallest_2->frequency;
                node->left = std::move(smallest_1);
                node->right = std::move(smallest_2);

                heap.emplace_back(std::move(node));
                std::push_heap(heap.begin(), heap.end(), std::greater<>{});
            }
        }

        auto node = std::make_unique<encoderTree>();
        node->character = '\0';
        node->frequency = 0;
        return node;
    }

    void build_encoder_table_rec(encoderTable& table, const std::unique_ptr<encoderTree>& root, encodedCharacter prefix, bool is_left_child);

    inline void build_encoder_table(encoderTable& table, const std::unique_ptr<encoderTree>& root) {
        build_encoder_table_rec(table, root, encodedCharacter(), false);
    }

    void build_encoder_table_rec(encoderTable& table, const std::unique_ptr<encoderTree>& root, encodedCharacter prefix, bool is_left_child) {
        prefix.append_bit(is_left_child);

        if (root->character != '\0') {
            table.get_mut(root->character) = prefix;
        }

        build_encoder_table_rec(table, root->left, prefix, true);
        build_encoder_table_rec(table, root->right, prefix, false);
    }

    bool operator<(const encoderTree& lhs, const encoderTree& rhs) {
        return lhs.frequency < rhs.frequency;
    }
}

namespace huffman::encoder
{
    encodedCharacter::encodedCharacter() : bits(0), code(0) {}

    void encodedCharacter::append_bit(bool bit) {
        auto bit_pos = bits % 8;
        if (bit_pos == 0) {
            code.push_back(static_cast<byte>(bit) << 7);
        } else {
            code.back() |= static_cast<byte>(bit) << (7 - bit_pos);
        }

        bits += 1;
    }

    encoderTable::encoderTable() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            table[i] = encodedCharacter();
        }
    }

    encoderTable::encoderTable(const std::string& text)
    {
        auto frequencies = detail::extract_frequencies(text);
        auto tree = detail::build_encoder_tree(frequencies);
        detail::build_encoder_table(*this, tree);
    }

    const encodedCharacter& encoderTable::get(char character) const {
        return table[static_cast<byte>(character)];
    }

    encodedCharacter& encoderTable::get_mut(char character) {
        return table[static_cast<byte>(character)];
    }

    //serialization and deserialization of the table
    std::vector<byte> encoderTable::serialize() const {
        auto serialized = std::vector<byte>();

        for (size_t i = 0; i < TABLE_SIZE; i++) {
            if (get(i).bits > 0) {
                auto serialized_character = detail::serialize_character(*this, i);
                serialized.insert(
                    std::end(serialized),
                    std::begin(serialized_character),
                    std::end(serialized_character)
                );
            }
        }

        return serialized;
    }

    encoderTable::encoderTable(const std::vector<byte>& encoded_table)
    {
        auto iter = encoded_table.cbegin();

        while (iter != encoded_table.cend()) {
            char character = 0;
            auto encoding = detail::deserialize_character(iter, character);
            
            table[static_cast<byte>(character)] = encoding;
        }
    }
}