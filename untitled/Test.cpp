#include "Function.h"


class Test{
public:
    void test_merge_words(){
        if (merge_words("word1","word2")=="word1word2"){
            std::cout << "test_merge_words passed";
        }
        else std::cout << "test_merge_words failed";
    }
    void test_first_letter(){
        if (first_letter("word")=="w"){
            std::cout << "test_first_letter passed";
        }
        else std::cout << "test_first_letter failed";
    }
    void test_last_letter(){
        if (last_letter("word")=="d"){
            std::cout << "test_last_letter passed";
        }
        else std::cout << "test_last_letter failed";
    }
    void test_add_word(){
        if (add_word("word1","word2")=="word1 word2"){
            std::cout << "test_add_word passed";
        }
        else std::cout << "test_add_word failed";
    }
    void test_all(){
        test_add_word();
        test_first_letter();
        test_last_letter();
        test_merge_words();
    }
};
