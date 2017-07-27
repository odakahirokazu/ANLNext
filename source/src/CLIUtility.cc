/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#include "CLIUtility.hh"

#if ANLNEXT_USE_READLINE
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <functional>
#include <readline/readline.h>
#include <readline/history.h>
#endif /* ANLNEXT_USE_READLINE */


#if ANLNEXT_USE_READLINE

namespace {

std::vector<std::string> g_CompletionCandidates;
char** completion_function(const char *text, int /* start */, int /* end */);
char* completion_key_generator(const char* text, int state);

} /* anonymous namespace */

namespace anlnext
{

ReadLine::ReadLine()
{
  rl_attempted_completion_function = nullptr;
  g_CompletionCandidates.clear();
}

ReadLine::~ReadLine()
{
  std::free(line_);
  rl_attempted_completion_function = nullptr;
  g_CompletionCandidates.clear();
}

int ReadLine::read(const char* prompt, const bool history, const bool trim_right)
{
  line_ = readline(prompt);
  if (line_ == nullptr) {
    return -1;
  }

  if (trim_right) {
    int pos = -1;
    for (int i=0; ; i++) {
      const int c = line_[i];
      if (c=='\0') { line_[pos+1]='\0'; break; }
      if (!std::isspace(c)) { pos = i; }
    }
  }

  int count = std::strlen(line_);
  if (history && count > 0) {
    add_history(line_);
  }
  return count;
}

void ReadLine::set_completion_candidates(const std::vector<std::string>& keys)
{
  g_CompletionCandidates = keys;
  rl_attempted_completion_function = completion_function;
}

} /* namespace anlnext */

namespace
{

char** completion_function(const char* text, int, int)
{
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, completion_key_generator);
}

char* completion_key_generator(const char* text, int state)
{
  static std::size_t keyIndex = 0;
  
  if (state==0) {
    keyIndex = 0;
  }

  while (keyIndex < g_CompletionCandidates.size()) {
    const std::string& key = g_CompletionCandidates[keyIndex++];
    if (key.find(text) == 0) {
      return strdup(key.c_str());
    }
  }
  return NULL; //nullptr;
}

} /* anonymous namespace */

#endif /* ANLNEXT_USE_READLINE */
