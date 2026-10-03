#pragma once

#include "raft_state.h"
#include "election_timer.h"
#include <memory>
#include <iostream>

class RaftNode {
public:
    RaftNode(int nodeId);

    void start();
    void stop();

    RaftState& getState();

private:
    void onElectionTimeout();

    RaftState state_;
    std::unique_ptr<ElectionTimer> timer_;
};