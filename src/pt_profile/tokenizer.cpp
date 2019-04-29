#include "pt_profile/tokenizer.h"

#include <algorithm>
#include <stdexcept>

namespace pt_profile
{
  Tokenizer::Tokenizer (std::string string, char separator)
    : m_string (std::move (string)),
      m_separator (separator),
      m_iterator (m_string.begin ())
  {
  }

  std::string Tokenizer::next ()
  {
    if (m_iterator == m_string.end ())
    {
      throw std::runtime_error ("No more tokens");
    }

    auto next_it = std::find (m_iterator, m_string.end (), m_separator);

    auto ret = std::string {m_iterator, next_it};

    m_iterator = std::find_if (next_it,
                               m_string.end (),
                               [this] (char c) { return c != m_separator; });

    return ret;
  }
}
