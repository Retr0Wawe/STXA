#include "identifier.hpp"
#include "lexer.hpp"

#include <algorithm>
#include <iostream>

namespace stxa
{
    Lexer::Lexer() : m_token_data({Token::T_NULL, 0, {}}), m_last_char(0)
    {   }

    Lexer::Lexer(const std::string &t_file_name) noexcept : 
    m_fstream(t_file_name), m_token_data({Token::T_NULL, 0, {}}), m_last_char(0)
    {   }

    // Parse keywords
    auto Lexer::findKeyword(std::string &t_identifier) -> bool
    {
        if (std::isalpha(m_last_char)) {
            t_identifier = m_last_char;
            while ((m_next_char = m_fstream.peek()) != EOF && m_next_char != '\n' && m_next_char != ' ') {
                if (std::isalnum((m_last_char = m_fstream.get()))) {
                    t_identifier.push_back(m_last_char);
                }
            }
            return true;
        }
        return false;
    }

    // Parse number
    auto Lexer::findNumber(std::string& t_identifier) -> bool
    {
        if (std::isdigit(m_last_char)) {
            m_token_data.m_file_ptr_pos = m_fstream.tellg() - std::streampos(1);   // Get start number position
            t_identifier = m_last_char;
            while ((m_next_char = m_fstream.peek()) != EOF && m_next_char != '\n' && m_next_char != ' ') {
                if (std::isdigit((m_last_char = m_fstream.get())) || m_last_char == '.') {
                    t_identifier.push_back(m_last_char);   // Push number character
                }
            }
            m_token_data.m_data = std::strtod(t_identifier.c_str(), nullptr);   // String to double
            return true;
        }
        return false;
    }

    // Parse comment
    auto Lexer::findComment() -> bool
    {
        if (m_last_char == '/') {    // Parse comment for skip it, but save read ptr position
            while ((m_next_char = m_fstream.peek()) != EOF && 
                    m_next_char != '\n' && m_next_char != '\r') 
            {
                m_last_char = m_fstream.get();
                continue;
            }
            return true;
        }
        return false;
    }
    
    // Overload operator() for checking istream object
    Lexer::operator bool() const noexcept
    {
        return m_fstream.is_open();
    }

    // Open file for parse
    auto Lexer::openFile(const std::string &t_file_name) noexcept -> Code
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
        while (m_fstream) {
            if (m_next_char == EOF || m_fstream.peek() == EOF) {    // Parse end of file
                m_token_data.m_file_ptr_pos = 0;
                m_token_data.m_data = {};
                return Token::T_EOF;
            }

            m_last_char = m_fstream.get();
            if (std::isspace(m_last_char)) {    // Skip space
                continue; 
            }

            if (findKeyword(m_identifier)) {    // Parse keyword
                auto find_tok = identifiers_en.find(m_identifier);
                if (find_tok != identifiers_en.end()) {
                    /* Finding the position of the token by calculating the string of the m_identifier and the position
                    in the file, thereby finding the beginning of the position of the token in the file */
                    if (m_next_char == EOF) {
                        m_fstream.seekg(0, std::ios::end);   // Go to end file
                        m_token_data.m_file_ptr_pos =   // Calculate position of m_identifier
                            m_fstream.tellg() - static_cast<std::streampos>(m_identifier.length());
                    } else {
                        m_token_data.m_file_ptr_pos =
                            m_fstream.tellg() - static_cast<std::streampos>(m_identifier.length());
                    }
                    return (m_token_data.m_token = find_tok->second);   // Return token
                }
            }

            if (findNumber(m_identifier)) {   // Parse number
                if (std::count_if(m_identifier.begin(), m_identifier.end(),
                    [&](char &c) { return c == '.'; }) > 1)   // Count dots in string
                {
                    m_token_data.m_data = {};   // Nulling std::variant
                    return Token::T_ERROR;      // If find number with two and more points, return error
                }
                return Token::T_NUMBER;
            }

            if (findComment()) {    // Parse comment for skip it, but save read ptr position
                m_token_data.m_file_ptr_pos = 0;
                return Token::T_COMMENT;
            }
        }

        return Token::T_NULL;    // Return null if no aviable another token values
    }

    auto Lexer::getLastTokenData() const -> const TokenData &
    {
        return m_token_data;   // Get token data(token, value(std::variant<double, string>), file ptr position)
    }
}
