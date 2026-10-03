#include "raft_node.h"

RaftNode::RaftNode(int nodeId)
    : state_(nodeId) {
    // Randomized timeout range: 150-300ms, standard Raft starting point.
    timer_ = std::make_unique<ElectionTimer>(150, 300, [this]() {
        onElectionTimeout();
    });
}

void RaftNode::start() {
    timer_->start();
}

void RaftNode::stop() {
    timer_->stop();
}

RaftState& RaftNode::getState() {
    return state_;
}

void RaftNode::onElectionTimeout() {
    // Leaders don't run elections against themselves — if we're already
    // leader, a timeout firing shouldn't do anything (we'll refine this
    // once heartbeats are wired in V4's next step).
    if (state_.getRole() == NodeRole::LEADER) {
        return;
    }

    std::cout << "[Node " << state_.getNodeId() << "] Election timeout — becoming candidate, term "
              << (state_.getCurrentTerm() + 1) << std::endl;

    state_.becomeCandidate();

    // NOTE: actually requesting votes from peers over the network comes next.
    // For now, this just proves the timeout correctly triggers the state transition.
}
