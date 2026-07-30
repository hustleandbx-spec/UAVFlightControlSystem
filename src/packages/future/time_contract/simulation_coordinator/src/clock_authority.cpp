#include "simulation_coordinator/clock_authority.hpp"

#include <limits>
#include <stdexcept>

namespace simulation_coordinator
{
ClockAuthority::ClockAuthority(const SimTimeNs physics_dt_ns)
: physics_dt_ns_(physics_dt_ns)
{
  if (physics_dt_ns_ <= 0) {
    throw std::invalid_argument("physics_dt_ns must be greater than zero");
  }
}

bool ClockAuthority::reset()
{
  if (generation_ == std::numeric_limits<GenerationId>::max()) {
    return false;
  }

  ++generation_;

  committed_step_ = 0;
  committed_time_ns_ = 0;

  pending_step_ = 0;
  pending_time_ns_ = 0;

  state_ = AuthorityState::READY;
  return true;
}

bool ClockAuthority::prepare_next_step()
{
  if (state_ != AuthorityState::READY) {
    return false;
  }

  if (committed_step_ == std::numeric_limits<StepId>::max()) {
    return false;
  }

  if (committed_time_ns_ > std::numeric_limits<SimTimeNs>::max() - physics_dt_ns_)
  {
    return false;
  }

  pending_step_ = committed_step_ + 1;
  pending_time_ns_ = committed_time_ns_ + physics_dt_ns_;

  state_ = AuthorityState::STEP_PENDING;
  return true;
}

bool ClockAuthority::commit_pending_step()
{
  if (state_ != AuthorityState::STEP_PENDING) {
    return false;
  }

  committed_step_ = pending_step_;
  committed_time_ns_ = pending_time_ns_;

  pending_step_ = 0;
  pending_time_ns_ = 0;

  state_ = AuthorityState::READY;
  return true;
}

void ClockAuthority::abort()
{
  pending_step_ = 0;
  pending_time_ns_ = 0;
  state_ = AuthorityState::ABORTED;
}

GenerationId ClockAuthority::generation() const
{
  return generation_;
}

StepId ClockAuthority::committed_step() const
{
  return committed_step_;
}

SimTimeNs ClockAuthority::committed_time_ns() const
{
  return committed_time_ns_;
}

StepId ClockAuthority::pending_step() const
{
  return pending_step_;
}

SimTimeNs ClockAuthority::pending_time_ns() const
{
  return pending_time_ns_;
}

SimTimeNs ClockAuthority::physics_dt_ns() const
{
  return physics_dt_ns_;
}

AuthorityState ClockAuthority::state() const
{
  return state_;
}

}  // namespace simulation_coordinator