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

#ifndef ANL_LoopCounter_H
#define ANL_LoopCounter_H 1

#include "ANLStatus.hh"

namespace anl
{

/**
 * Loop counter
 *
 * @author Hirokazu Odaka
 * @date 2017-07-02 | based on struct ANLModuleCounter
 */
class LoopCounter
{
public:
  LoopCounter() = default;
  ~LoopCounter() = default;
  LoopCounter(const LoopCounter&) = default;
  LoopCounter(LoopCounter&&) = default;
  LoopCounter& operator=(const LoopCounter&) = default;
  LoopCounter& operator=(LoopCounter&&) = default;

  long int entry() const { return entry_; }
  long int ok() const { return ok_; }
  long int error() const { return error_; }
  long int skip() const { return skip_; }
  long int quit() const { return quit_; }
  
  void reset()
  {
    entry_ = 0;
    ok_ = 0;
    error_ = 0;
    skip_ = 0;
    quit_ = 0;
  }

  void count_up_by_entry()
  {
    ++entry_;
  }

  void count_up_by_result(ANLStatus status)
  {
    if (status == AS_OK) {
      ++ok_;
    }
    else if (status == AS_SKIP) {
      ++skip_;
    }
    else if (status == AS_SKIP_ERROR) {
      ++skip_;
      ++error_;
    }
    else if (status == AS_QUIT || status == AS_QUIT_ALL) {
      ++quit_;
    }
    else if (status == AS_QUIT_ERROR || status == AS_QUIT_ALL_ERROR) {
      ++quit_;
      ++error_;
    }
  }

  LoopCounter operator+(const LoopCounter& r) const
  {
    LoopCounter a(*this);
    a.entry_ += r.entry_;
    a.ok_    += r.ok_;
    a.error_ += r.error_;
    a.skip_  += r.skip_;
    a.quit_  += r.quit_;
    return a;
  }

  LoopCounter& operator+=(const LoopCounter& r)
  {
    *this = *this + r;
    return *this;
  }

private:
  long int entry_ = 0;
  long int ok_ = 0;
  long int error_ = 0;
  long int skip_ = 0;
  long int quit_ = 0;
};

} /* namespace anl */

#endif /* ANL_LoopCounter_H */
