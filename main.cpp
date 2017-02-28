#include <stdio.h>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <iostream>

bool readFileText(std::string const& filePath, std::string& out_fileText) {
	std::ifstream file(filePath, std::ios_base::ate);
	if(!file.is_open()) return false;

	size_t fileSize = file.tellg();
	out_fileText = std::string(fileSize, ' ');
	file.seekg(0, file.beg);

	file.read((char*)((void*)out_fileText.data()), fileSize);
	file.close();
	return true;
}

struct Token1 {
	enum class Type : uint8_t {
		Word = 0,
		String = 1,
		Number = 2,
		OpenBracket,
		CloseBracket,
		MathOp
	};
	
	static const std::string TYPE_STRING[6];
	
	Token1::Type type;
	std::string text;
	
	Token1(Token1::Type const& type_, std::string const& text_) : type(type_), text(text_) {}
};

const std::string Token1::TYPE_STRING[6] = {
	"word",
	"string",
	"number",
	"open_bracket",
	"close_bracket",
	"math_op"
};

typedef std::vector<Token1> Token1List;

void printToken1List(Token1List const& tokens, std::ostream& out = std::cout) {
	out << "Tokens:" << std::endl;
	for(size_t i = 0, n = tokens.size(); i < n; ++i)
		out << i << " " << Token1::TYPE_STRING[(int)tokens[i].type] << " " << tokens[i].text << std::endl;
}

typedef std::vector<std::string> ErrorList;

void printErrorList(ErrorList const& errors, std::ostream& out = std::cout) {
	out << "Errors:" << std::endl;
	for(size_t i = 0, n = errors.size(); i < n; ++i)
		out << i << " " << errors[i] << std::endl;
}

const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string NUMBERS = "0123456789";
const std::string DELIMETERS = ".;,()[]{}\\/@: ";
const std::string MATH_OPS = "+-*/\\%";

bool tokenizeText_Number(std::string const& text, Token1List& out_tokens, ErrorList& out_errors, size_t& text_index);
bool tokenizeText_String(std::string const& text, Token1List& out_tokens, ErrorList& out_errors, size_t& text_index);

bool tokenizeText(std::string const& text, Token1List& out_tokens, ErrorList& out_errors, size_t& text_index) {
	// Skip empty space
	size_t pos = text.find_first_not_of(" \n", text_index);
	if(std::string::npos == pos)
		return true;
	text_index = pos;
	
	const char ch = text[text_index];
	
	// Found number
	if(std::string::npos != NUMBERS.find(ch)) {
		if(!tokenizeText_Number(text, out_tokens, out_errors, text_index))
			return false;
	}
	
	// Found string
	else if(ch == '\'' || ch == '"') {
		if(!tokenizeText_String(text, out_tokens, out_errors, text_index))
			return false;
	}
	
	// TODO other brackets
	
	else if(ch == '(') {
		out_tokens.emplace_back(Token1::Type::OpenBracket, "(");
		text_index++;
	}
	
	else if(ch == ')') {
		out_tokens.emplace_back(Token1::Type::CloseBracket, ")");
		text_index++;
	}
	
	// Found math op
	else if(std::string::npos != MATH_OPS.find(ch)) {
		std::string word = text.substr(text_index, 1);
		out_tokens.emplace_back(Token1::Type::MathOp, word);
		text_index++;
	}
	
	// Word
	else {
		size_t end_of_word = text.find_first_of(DELIMETERS, text_index+1);
		
		// if end of text
		if(std::string::npos == end_of_word) {
			std::string word = text.substr(text_index);
			out_tokens.emplace_back(Token1::Type::Word, word);
			text_index = text.size();
			return true;
		}
		
		std::string word = text.substr(text_index, end_of_word - text_index);
		out_tokens.emplace_back(Token1::Type::Word, word);
		text_index = end_of_word;
	}
	
	return tokenizeText(text, out_tokens, out_errors, text_index);
}

bool tokenizeText(std::string const& text, Token1List& out_tokens, ErrorList& out_errors) {
	size_t text_index = 0;
	return tokenizeText(text, out_tokens, out_errors, text_index);
}
	   
bool tokenizeText_Number(std::string const& text, Token1List& out_tokens, ErrorList& out_errors, size_t& text_index) {
	size_t pos = text.find_first_not_of(NUMBERS, text_index);
	std::string word = text.substr(text_index, pos - text_index);
	text_index = pos;
	out_tokens.emplace_back(Token1::Type::Number, word);
	return true;
}

bool tokenizeText_String(std::string const& text, Token1List& out_tokens, ErrorList& out_errors, size_t& text_index) {
	// Current string type delim
	char delim = text[text_index];
	
	size_t string_end = text.find(delim, text_index + 1);
	if(std::string::npos == string_end) {
		out_errors.push_back("end of string not found at " + std::to_string(text_index));
		return false;
	}
	
	std::string word = text.substr(text_index, string_end - text_index + 1);
	out_tokens.emplace_back(Token1::Type::String, word);
	text_index = string_end + 1;
	return true;
}

int main() {
	std::string hello_text;
    if(!readFileText("./hello.js", hello_text)) {
		puts("failed read ./hello.js");
		return 1;
	}
	puts("file readed");
	
	Token1List tokens;
	ErrorList errors;
	
	if(!tokenizeText(hello_text, tokens, errors)) {
		std::cout << "failed parse tokens from this source:" << std::endl << hello_text << std::endl;
		printErrorList(errors);
		return 1;
	}
	puts("tokens parsed");
	
	printToken1List(tokens);
	
    return 0;
}
