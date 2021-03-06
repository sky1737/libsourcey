///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <http://sourcey.com>
//
// SPDX-License-Identifier: LGPL-2.1+
//
/// @addtogroup base
/// @{
// This file uses the public domain libb64 library:
// http://libb64.sourceforge.net/
//


#ifndef SCY_Hex_H
#define SCY_Hex_H


#include "scy/error.h"
#include "scy/interface.h"
#include "scy/logger.h"
#include <assert.h>
#include <cstring>
#include <iostream>


namespace scy {
namespace hex {


//
// Hex Encoder
//


/// Hex encoder.
struct Encoder : public basic::Encoder
{
    Encoder()
        : _linePos(0)
        , _lineLength(72)
        , _uppercase(0)
    {
    }

    virtual ssize_t encode(const char* inbuf, size_t nread, char* outbuf)
    {
        // static const int eof = std::char_traits<char>::eof();
        static const char digits[] = "0123456789abcdef0123456789ABCDEF";

        char c;
        size_t nwrite = 0;
        for (unsigned i = 0; i < nread; i++) {
            c = inbuf[i];
            std::memcpy(outbuf + nwrite++, &digits[_uppercase + ((c >> 4) & 0xF)], 1);
            std::memcpy(outbuf + nwrite++, &digits[_uppercase + (c & 0xF)], 1);
            if (_lineLength > 0 &&
                (_linePos += 2) >= _lineLength) { //++_linePos//++_linePos;
                _linePos = 0;
                std::memcpy(outbuf + nwrite++, "\n", 1);
            }
        }

        return nwrite;
    }

    virtual ssize_t finalize(char* /* outbuf */) { return 0; }

    void setUppercase(bool flag) { _uppercase = flag ? 16 : 0; }

    void setLineLength(int lineLength) { _lineLength = lineLength; }

    int _linePos;
    int _lineLength;
    int _uppercase;
};


/// Converts the STL container to Hex.
template <typename T>
inline std::string encode(const T& bytes)
{
    static const char digits[] = "0123456789abcdef";
    std::string res;
    res.reserve(bytes.size() * 2);
    for (typename T::const_iterator it = bytes.begin(); it != bytes.end();
         ++it) {
        const unsigned char c = static_cast<const unsigned char>(*it);
        res += digits[(c >> 4) & 0xF];
        res += digits[c & 0xF];
    }
    return res;
}


//
// Hex Decoder
//


/// Hex decoder.
struct Decoder : public basic::Decoder
{
    Decoder()
        : lastbyte('\0')
    {
    }
    virtual ~Decoder() {}

    virtual ssize_t decode(const char* inbuf, size_t nread,
                               char* outbuf)
    {
        int n;
        char c;
        size_t rpos = 0;
        size_t nwrite = 0;
        while (rpos < nread) {
            if (readnext(inbuf, nread, rpos, c))
                n = (nybble(c) << 4);

            else if (rpos >= nread) {
                // Store the last byte to be
                // prepended on next decode()
                if (!iswspace(inbuf[rpos - 1]))
                    std::memcpy(&lastbyte, &inbuf[rpos - 1], 1);
                break;
            }

            readnext(inbuf, nread, rpos, c);
            n = n | nybble(c);
            std::memcpy(outbuf + nwrite++, &n, 1);
        }
        return nwrite;
    }

    virtual ssize_t finalize(char* /* outbuf */) { return 0; }

    bool readnext(const char* inbuf, size_t nread, size_t& rpos,
                  char& c)
    {
        if (rpos == 0 && lastbyte != '\0') {
            assert(!iswspace(lastbyte));
            c = lastbyte;
            lastbyte = '\0';
        } else {
            c = inbuf[rpos++];
            while (iswspace(c) && rpos < nread)
                c = inbuf[rpos++];
        }
        return rpos < nread;
    }

    int nybble(const int n)
    {
        if (n >= '0' && n <= '9')
            return n - '0';
        else if (n >= 'A' && n <= 'F')
            return n - ('A' - 10);
        else if (n >= 'a' && n <= 'f')
            return n - ('a' - 10);
        else
            throw std::runtime_error("Invalid hex format");
    }

    bool iswspace(const char c)
    {
        return c == ' ' || c == '\r' || c == '\t' || c == '\n';
    }

    char lastbyte;
};


} // namespace hex
} // namespace scy


#endif // SCY_Hex_H


/// @\}
