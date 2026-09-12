#include "raft_state.h"

RaftState::RaftState(int nodeId)
    : nodeId_(nodeId), currentTerm_(0), role_(NodeRole::FOLLOWER), votedFor_(-1) {
}

int RaftState::getCurrentTerm() {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentTerm_;
}

NodeRole RaftState::getRole() {
    std::lock_guard<std::mutex> lock(mutex_);
    return role_;
}

int RaftState::getNodeId() const {
    return nodeId_;
}

void RaftState::stepDownToFollower(int newTerm) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentTerm_ = newTerm;
    role_ = NodeRole::FOLLOWER;
    votedFor_ = -1;  // new term means we haven't voted yet in it
}

void RaftState::becomeCandidate() {
    std::lock_guard<std::mutex> lock(mutex_);
    currentTerm_ += 1;
    role_ = NodeRole::CANDIDATE;
    votedFor_ = nodeId_;  // vote for self
}

void RaftState::becomeLeader() {
    std::lock_guard<std::mutex> lock(mutex_);
    role_ = NodeRole::LEADER;
}

void RaftState::becomeFollower() {
    std::lock_guard<std::mutex> lock(mutex_);
    role_ = NodeRole::FOLLOWER;
}

bool RaftState::tryGrantVote(int candidateTerm, int candidateId) {
    std::lock_guard<std::mutex> lock(mutex_);

    // If the candidate's term is behind ours, reject immediately.
    if (candidateTerm < currentTerm_) {
        return false;
    }

    // If the candidate's term is ahead of ours, we're outdated —
    // update our term and reset our vote before considering this request.
    if (candidateTerm > currentTerm_) {
        currentTerm_ = candidateTerm;
        role_ = NodeRole::FOLLOWER;
        votedFor_ = -1;
    }

    // Grant the vote only if we haven't already voted this term
    // (or we already voted for this exact candidate — handles duplicate requests).
    if (votedFor_ == -1 || votedFor_ == candidateId) {
        votedFor_ = candidateId;
        return true;
    }

    return false;
}