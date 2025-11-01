#include <unordered_map>
#include <string>
#include <string_view>
#include <gtest/gtest.h>

namespace
{
    class SuffixTrie
    {
    public :
        void insert(std::string word)
        {
            insert(_root, word, true);

            for (int n = 1; n < word.size(); ++n)
            {
                insert(_root,
                       std::string_view(word.data() + n, word.size() - n),
                       false);
            }
        }

        [[nodiscard]]
        inline bool search(std::string word) const
        {
            return search(_root, word);
        }

        [[nodiscard]]
        inline bool endsWith(std::string suffix) const
        {
            return endsWith(_root, suffix);
        }

    private :
        struct Node
        {
            char c = '\0';
            bool isTerminal = false;
            std::unordered_map<char, Node *> childNodes;
        };

        Node *_root = new Node{'\0', true};

        void insert(Node *node, std::string_view word, bool isTerminal)
        {
            if (word.empty())
            {
                node->isTerminal = isTerminal;

                return;
            }

            auto it = node->childNodes.find(word[0]);

            if (it == node->childNodes.end())
            {
                it = node->childNodes.emplace(
                    word[0], new Node{word[0]}).first;
            }

            insert(it->second, word.substr(1), isTerminal);
        }

        [[nodiscard]]
        bool search(Node *node, std::string_view word) const
        {
            if (word.empty())
            {
                return node->isTerminal;
            }

            auto it = node->childNodes.find(word[0]);

            return (it != node->childNodes.cend()) ?
                search(it->second, word.substr(1)) : false;
        }

        [[nodiscard]]
        bool endsWith(Node *node, std::string_view suffix) const
        {
            if (suffix.empty())
            {
                return !node->isTerminal;
            }

            auto it = node->childNodes.find(suffix[0]);

            return (it != node->childNodes.cend()) ?
                endsWith(it->second, suffix.substr(1)) : false;
        }
    };
}

TEST(SuffixTrie, Test_1)
{
    SuffixTrie st;

    st.insert("banana");
    st.insert("bad");
    st.insert("boss");

    EXPECT_FALSE(st.search("anana"));
    EXPECT_FALSE(st.search("d"));
    EXPECT_FALSE(st.search("ss"));
    EXPECT_TRUE(st.search("banana"));
    EXPECT_TRUE(st.search("bad"));
    EXPECT_TRUE(st.search("boss"));

    EXPECT_TRUE(st.endsWith("anana"));
    EXPECT_TRUE(st.endsWith("d"));
    EXPECT_TRUE(st.endsWith("ss"));
    EXPECT_FALSE(st.endsWith("banana"));
    EXPECT_FALSE(st.endsWith("bad"));
    EXPECT_FALSE(st.endsWith("boss"));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
