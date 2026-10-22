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

#ifndef ANLNEXT_OrderKeeper_H
#define ANLNEXT_OrderKeeper_H 1

#include <cstddef>
#include <mutex>
#include <condition_variable>

namespace anlnext
{

/**
 * OrderKeeper
 *
 * @author Hirokazu Odaka
 * @date 2017-07-12
 */
class OrderKeeper
{
public:
  OrderKeeper() = default;
  ~OrderKeeper() = default;
  OrderKeeper(const OrderKeeper&) = delete;
  OrderKeeper(OrderKeeper&&) = delete;
  OrderKeeper& operator=(const OrderKeeper&) = delete;
  OrderKeeper& operator=(OrderKeeper&&) = delete;

  // False means that the run was cancelled; the caller must not execute.
  bool wait(std::size_t index)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this, index](){ return !normal_ || index == next_index_; });
    return normal_;
  }

  void send_done(std::size_t index)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (normal_) {
      next_index_ = index + 1;
    }
    cv_.notify_all();
  }

  void cancel()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    normal_ = false;
    cv_.notify_all();
  }

  // Only call between runs, after all workers have joined.
  void reset()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    next_index_ = 0;
    normal_ = true;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::size_t next_index_ = 0;
  bool normal_ = true;  // Normal sequencing; false after cancel().
};

template<typename KeeperType, typename IndexType>
class KeeperBlock
{
public:
  KeeperBlock(KeeperType* keeper, IndexType index)
    : keeper_(keeper), index_(index)
  {
    if (keeper) {
      can_proceed_ = keeper->wait(index);
    }
  }

  ~KeeperBlock()
  {
    if (keeper_ && can_proceed_) {
      keeper_->send_done(index_);
    }
  }

  KeeperBlock(const KeeperBlock&) = delete;
  KeeperBlock& operator=(const KeeperBlock&) = delete;
  // Result of the initial wait (true without a keeper), not a live cancellation check.
  bool can_proceed() const { return can_proceed_; }

private:
  bool can_proceed_ = true;
  KeeperType* keeper_;
  IndexType index_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_OrderKeeper_H */
