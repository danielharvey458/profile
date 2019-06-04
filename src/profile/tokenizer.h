#include <string>

namespace profile
{
  class Tokenizer
  {
    public:

      explicit Tokenizer (std::string string, char separator);

      std::string next ();

    private:
      std::string m_string;
      char m_separator = {};
      std::string::iterator m_iterator;
  };
}
