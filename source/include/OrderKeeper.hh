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

  void wait(long int index)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [=](){ return (index-1 == last_done_index_); });
  }

  void send_done(long int index)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    last_done_index_ = index;
    cv_.notify_all();
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  long int last_done_index_ = -1;
};

template<typename KeeperType, typename IndexType>
class KeeperBlock
{
public:
  KeeperBlock(KeeperType* keeper, IndexType index)
    : keeper_(keeper), index_(index)
  {
    if (keeper) {
      keeper->wait(index);
    }
  }

  ~KeeperBlock()
  {
    if (keeper_) {
      keeper_->send_done(index_);
    }
  }

private:
  KeeperType* keeper_;
  IndexType index_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_OrderKeeper_H */
