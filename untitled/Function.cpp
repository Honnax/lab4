#include <iostream>
std::string merge_words(std::string word1,std::string word2){
    return word1+word2;
}
std::string first_letter(std::string word){
    return word.substr(0,1);
}
std::string last_letter(std::string word){
    return word.substr(word.length()-1,1);
}
std::string add_word(std::string word1, std::string word2){
    return word1+" "+word2;
}
