#include "encoder_table.h"

#include <algorithm>
#include <unordered_map>
#include <memory>

#include "serializable_character.h"
#include "../utils.h"

namespace huffman::encoder::detail
{
    struct encoderTree {
        char character;
        int frequency;
        std::unique_ptr<encoderTree> left, right;
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

    bool heap_compare(const std::unique_ptr<encoderTree>& lhs, const std::unique_ptr<encoderTree>& rhs) {
        return lhs->frequency > rhs->frequency || (lhs->frequency == rhs->frequency && lhs->character < rhs->character);
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

        std::make_heap(heap.begin(), heap.end(), heap_compare);

        while (!heap.empty()) {
            std::pop_heap(heap.begin(), heap.end(), heap_compare);
            auto smallest_1 = std::move(heap.back()); heap.pop_back();

            if (heap.empty()) {
                return smallest_1;
            } else {
                std::pop_heap(heap.begin(), heap.end(), heap_compare);
                auto smallest_2 = std::move(heap.back()); heap.pop_back();

                auto node = std::make_unique<encoderTree>();
                node->character = '\0';
                node->frequency = smallest_1->frequency + smallest_2->frequency;
                node->left = std::move(smallest_1);
                node->right = std::move(smallest_2);

                heap.emplace_back(std::move(node));
                std::push_heap(heap.begin(), heap.end(), heap_compare);
            }
        }

        auto node = std::make_unique<encoderTree>();
        node->character = '\0';
        node->frequency = 0;
        return node;
    }

    void build_encoder_table_rec(encoderTable& table, const std::unique_ptr<encoderTree>& root, encodedCharacter prefix, bool is_right_child);

    inline void build_encoder_table(encoderTable& table, const std::unique_ptr<encoderTree>& root) {
        if (!root) return;

        build_encoder_table_rec(table, root->left, encodedCharacter(), false);
        build_encoder_table_rec(table, root->right, encodedCharacter(), true);
    }

    void build_encoder_table_rec(encoderTable& table, const std::unique_ptr<encoderTree>& root, encodedCharacter prefix, bool is_right_child) {
        if (!root) return;

        prefix.append_bit(is_right_child);

        if (root->character != '\0') {
            table.get_mut(root->character) = prefix;
        }

        build_encoder_table_rec(table, root->left, prefix, false);
        build_encoder_table_rec(table, root->right, prefix, true);
    }
}

namespace huffman::encoder
{
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

    //serialization and deserialization of the table
    std::vector<byte> encoderTable::serialize() const {
        auto serialized = std::vector<byte>();

        byte character_count = 0;
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            if (get(i).bits > 0) {
                auto serialized_character = serializableCharacter(i, get(i)).serialize();
                serialized.insert(
                    std::end(serialized),
                    std::begin(serialized_character),
                    std::end(serialized_character)
                );

                character_count++;
            }
        }

        serialized.insert(serialized.begin(), character_count);
        return serialized;
    }

    std::string encoderTable::to_string() const
    {
        auto out_string = std::string("[\n");

        for (size_t i = 0; i < TABLE_SIZE; i++) {
            auto& elem = get(i);
            if (elem.bits > 0) {
                out_string += "  " + std::string(1, static_cast<char>(i)) + " : " + elem.to_string() + "\n";
            }
        }

        out_string += "]";
        
        return out_string;
    }
}