#include "identifier.hpp"
#include "lexer.hpp"

#include <iostream>
#include <algorithm>

namespace stxa
{
    Lexer::Lexer(): m_token_data({Token::T_NULL, 0, {}})
    {   }

    Lexer::Lexer(const std::string& t_file_name) noexcept : m_token_data({Token::T_NULL, 0, {}}),
                                                            m_fstream(t_file_name)
    {   }

    Lexer::operator bool() const noexcept
    {
        return m_fstream.is_open();
    }

    // Open file for parse
    auto Lexer::openFile(const std::string& t_file_name) noexcept -> Code
    {
        if (m_fstream.is_open()) {
            m_fstream.close();
        }

        m_fstream.open(t_file_name, std::ios::in);

        if (!m_fstream.is_open()) {
            return Code::FILE_OPEN_ERROR;
        }

        return Code::SUCCES;
    }

    // Get tokens from file
    auto Lexer::getNextToken() -> Token
    {
        static char last_char = 0;

        while (m_fstream.get(last_char)) {
            std::string identifier;

            if (std::isspace(last_char)) {
                continue;
            }

            if (std::isalpha(last_char)) {
                identifier = last_char;

                while (std::isalnum((last_char = m_fstream.get()))) {
                    identifier.push_back(last_char);
                }

                auto find_tok = identifiers_en.find(identifier);
                if (find_tok != identifiers_en.end()) {
                    m_token_data.m_file_ptr_pos = m_fstream.tellg();
                    m_token_data.m_token = find_tok->second;
                    return  m_token_data.m_token;
                }
            }

            if (std::isdigit(last_char) || last_char == '.') {
                    std::string number_str;
                    do {
                        number_str.push_back(last_char);
                        last_char = m_fstream.get();
                    } while (std::isdigit(last_char) || last_char == '.');

                    if (std::count_if(number_str.begin(), number_str.end(), 
                       [&number_str](char &c) { return c == '.'; } ) > 1) 
                    {
                        m_token_data.data = {};   // Nulling std::variant
                        m_token_data.m_file_ptr_pos = m_fstream.tellg();   // Get token position
                        return Token::T_ERROR;   // If find number with two and more points, return error
                    }
                    m_token_data.m_file_ptr_pos = m_fstream.tellg();
                    m_token_data.data = std::strtod(number_str.c_str(), nullptr);   // String to double
                    return Token::T_NUMBER;
            }

            if (last_char == EOF) {
                m_token_data.data = {};
                return Token::T_EOF;
            }
        }

        return Token::T_NULL;
    }

    auto Lexer::getLastTokenData() const -> const TokenData&
    {
       return m_token_data;
    }
}
